//server.c
#include <sys/socket.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>

#include "server.h"

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
  //while(this->work)
  for (int i = 0 ; i < 5; i++)
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
        printf("Novy hrac %d a %p\n", new_player->fd, new_player);
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
  printf("new player %p created\n", player);
}
void player_task()
{

}
void destroy_player(void * data, void * in, void * out, void * err)
{ 
  struct player * this = *(struct player **)data;
  printf("destroy player %d a %p\n", this->fd, this);   

  pthread_join(this->thread, NULL);

  pthread_mutex_destroy(&this->mut_action);
  free(this);
}
void server_start(struct server* this)
{
  this->work = 1;
  server_connect_players(this);
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
  server_destroy(&s);

}
