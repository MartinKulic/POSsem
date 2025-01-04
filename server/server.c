//server.c
#include <sys/socket.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "../share/com_protocol.h"


int server_init(struct server* this, int port)
{
  // setup communication
  int opt = 1;
  this->MAX_PLAYERS = 5;
  
  if((this->server_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket failed");
    return 0;
  }

  if(setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR/* | SO_REUSEPORT*/, &opt, sizeof(opt)))
  {
    perror("setsockopt failde");
    return 0;
  }

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;
  this->address.sin_port = htons(port);

  if(bind(this->server_fd, (struct sockaddr*)&this->address, sizeof(this->address)) < 0)
  {
    perror("bind failed");
    return 0;
  }

  if(listen(this->server_fd, this->MAX_PLAYERS) < 0)
  {
    perror("puting socket to pasive mode failed");
    return 0;
  }

  //setup other
  this->players = calloc(1, sizeof(struct sll));
  sll_init(this->players, sizeof(struct player **));
  this->mut_players = calloc(1, sizeof(pthread_mutex_t));
  pthread_mutex_init(this->mut_players, NULL);
}

void * server_connect_players(void * arg)
{
  struct server * this = arg;
  struct pollfd fds[1];
  fds[0].fd = this->server_fd;
  fds[0].events = POLLIN;
  while(this->work)
  //for (int i = 0 ; i < 5; i++)
  {
    if(poll(fds, 1, 950)>0)
    {
      if(fds[0].revents == 0)
      {
        printf("toto by sa stat nemalo\n");
        continue;
      }
      if(fds[0].revents & POLLIN)
      {
        
        struct player * new_player = calloc(1, sizeof(player));
        socklen_t addrlen = sizeof(this->address);
        new_player->fd = accept(this->server_fd, (struct sockaddr*)&this->address, &addrlen);
        if(new_player->fd < 0)
        {
          printf("failed to make new socket\n");
          continue;
        }
        if(sll_get_size(this->players) > this->MAX_PLAYERS)
        {
          printf("novy hrac ale je plno\n");
          char * msg = "Plno";
          my_send(new_player->fd, msg);
       //   int conv_next_msg_size = htonl(strlen(msg));
       //   send(new_player->fd, &conv_next_msg_size, sizeof(conv_next_msg_size), 0);
       //   send(new_player->fd, msg, strlen(msg), 0);
          close(new_player->fd);
          continue;
        }
        printf("Novy hrac %d\n", new_player->fd);
        struct ser_pla* sp = calloc(1, sizeof(ser_pla));//{new_player, this};
        sp->player = new_player; //uvolni sa v player_init_a_dispache
        sp->server = this;
        pthread_create(&new_player->thread, NULL, player_init_a_dispache, sp);
        
        
      }else
      {
        printf("prisiel event ale nie POLL IN\n");
        continue;

      }
    }
  }
}
void * player_init_a_dispache(void * arg)
{
  struct ser_pla * sp = arg;
  struct server* server = sp->server;
  struct player* player = sp->player;

  free(sp);

  pthread_mutex_lock(server->mut_players);
  sll_add(server->players, &player);
  pthread_mutex_unlock(server->mut_players);
  
  pthread_mutex_init(&player->mut_action, NULL);
  player->id = player->fd;
  printf("new player %d created\n", player->id);

  //char * msg = "OK";
  my_send(player->fd, "OK");
//  int conv_next_msg_size = htonl(strlen(msg));
//  send(player->fd, &conv_next_msg_size, sizeof(conv_next_msg_size), 0);
//  send(player->fd, msg, strlen(msg), 0);
  player->work = 1;

  player_in_task(player);
}
void player_in_task(struct player * this)
{
  struct pollfd fds[1];
  fds[0].fd = this->fd;
  fds[0].events = POLLIN;

  while(this->work)
  {
    if(poll(fds, 1, 1000) > 0)
    {
      printf("player %d revent %d ", this->id, fds[0].revents);
      if(fds[0].revents & POLLIN)
      {
        char n_a[1];
        recv(this->fd, &n_a, 1, 0);

        pthread_mutex_lock(&this->mut_action);
        this->next_action = *n_a;
        pthread_mutex_unlock(&this->mut_action);

        
        printf("player %d recieved %c\n", this->id, this->next_action);
        if(this->next_action == 'q')
        {
          this->work = 0;
          break;
        }

        my_send(this->fd, "OK");
      }
    }
  }
}
void destroy_player(void * data, void * in, void * out, void * err)
{ 
  struct player * this = *(struct player **)data;
  //printf("destroy player %d a %p\n", this->fd, this);   

  this->work = 0;
  pthread_join(this->thread, NULL);
  pthread_mutex_destroy(&this->mut_action);

  close(this->fd);

  free(this);
}
void server_start(struct server* this)
{
  this->work = 1;
  pthread_t l_thread;
  pthread_create(&l_thread, NULL, server_logic,this);

  server_connect_players(this);
  close(this->server_fd);

  pthread_join(l_thread,NULL);
}

void* server_logic(void*arg)
{
  struct server* this = arg;
  _Bool no_players = 0; 

  time_t no_player_time_start = 0;
  time_t curr_time;
  while (this->work) {
  
    if(sll_get_size(this->players)==0)
    {
      if(no_player_time_start == 0)
      {
        time(&no_player_time_start);
      }
      time(&curr_time);
      if((curr_time-no_player_time_start) > NO_ACTIVY_SERVER_END)
      {
        this->work = 0;
      }
      printf("no players for %d s\n", (curr_time - no_player_time_start));
      
    }
    else
    {
      server_tick(this);
      if(no_player_time_start != 0)
      {
        no_player_time_start = 0;
      }
    } 
    sleep(1);
  }
}
void server_tick(struct server * this)
{
  sll index_endedPlayers;
  sll_init(&index_endedPlayers, sizeof(int));
  int index = 0;

  pthread_mutex_lock(this->mut_players);
  sll_for_each(this->players, &server_ack_player_next_action, NULL, NULL, NULL);
  pthread_mutex_unlock(this->mut_players);
    
  sll_for_each(this->players, &server_do_player_action, &index, &index_endedPlayers, NULL);

  if(sll_get_size(&index_endedPlayers) > 0)
  {
    pthread_mutex_lock(this->mut_players);
    sll_for_each(&index_endedPlayers, &remove_player_from_players, this->players, NULL, NULL);
    pthread_mutex_unlock(this->mut_players);
  }
  sll_clear(&index_endedPlayers);
}
void server_ack_player_next_action(void * data, void * in, void * out, void * err)
{
  struct player * player = *(struct player **) data;
   
  pthread_mutex_lock(&player->mut_action);
 
  player->action = player->next_action;
  
  pthread_mutex_unlock(&player->mut_action);
   
}
void server_do_player_action(void * data, void * in, void * out, void * err)
{
  struct player * player = *(struct player **) data;
  int * index = (int *)in;
  printf("\tplayer-%d-i>%d-na> %c -a> %c\n", player->id, *index, player->next_action, player->action);

  if(player->action == 'q'){
    destroy_player(&player,NULL,NULL,NULL);
    sll * index_p = out;
    sll_add(index_p, index);
  }

  *index = *index+1;
}
void remove_player_from_players(void * data, void * in, void * out, void * err)
{
  int index = *(int *)data;
  printf("removing player at %d\n", index);
  sll * players = in;
  sll_remove(players, index);
}


void server_destroy(struct server * this)
{
  this->work = 0;
  sll_for_each(this->players, &destroy_player, NULL, NULL, NULL);
  free(this->mut_players);

  sll_clear(this->players);
  free(this->players);
}
//--------------------------------------
int main (int argc, char* argv[])
{
  struct server s;
  server_init(&s, PORT);
  printf("server initialized\n");
  server_start(&s);
  printf("server ended\n");
  server_destroy(&s);

}
