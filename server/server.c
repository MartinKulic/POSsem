//server.c
#include <sys/socket.h>
#include <poll.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "../share/com_protocol.h"


int server_init(struct server* this, run_param * rp)
{
  // setup other
  this->MAX_PLAYERS = rp->max_players;
  coord max_map = (struct coord){rp->map_w,rp->map_h};

  this->players = calloc(1, sizeof(struct sll));
  sll_init(this->players, sizeof(struct player **));
  this->mut_players = calloc(1, sizeof(pthread_mutex_t));
  pthread_mutex_init(this->mut_players, NULL);
  this->mut_map = calloc(1, sizeof(pthread_mutex_t));
  pthread_mutex_init(this->mut_map, NULL);
  
  this->map = calloc(1, sizeof(struct map));
  if(rp->path_to_map[0] == '\0')
  {
    map_init(this->map, EMPTY_CELL, max_map);
  }
  else
  {
    if (map_init_from_file(this->map, rp->path_to_map) != 1)
    {
      this->map->dim = (coord){-1,-1};
      return 0;
    }
  
  }

  this->fruit_left=0;

  this->time_start = time(NULL);
  this->time_duration = rp->game_time;

  srand(time(NULL));
  //---------------------------------------
   // setup communication
  int opt = 1;

  if((this->server_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("\033[91msocket failed\033[0m");
    return 0;
  }

  if(setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR/* | SO_REUSEPORT*/, &opt, sizeof(opt)))
  {
    perror("\033[91setsockopt failde\033[0m");
    return 0;
  }

  this->address.sin_family = AF_INET;
  this->address.sin_addr.s_addr = INADDR_ANY;
  this->address.sin_port = htons(rp->port);

  if(bind(this->server_fd, (struct sockaddr*)&this->address, sizeof(this->address)) < 0)
  {
    perror("\033[91bind failed\033[0m");
    return 0;
  }

  if(listen(this->server_fd, this->MAX_PLAYERS) < 0)
  {
    perror("\033[91puting socket to pasive mode failed\033[0m");
    return 0;
  }
  //---------------------------------------
  
 
  return 1;
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
          //printf("failed to make new socket\n");
          continue;
        }
        if(sll_get_size(this->players) > this->MAX_PLAYERS)
        {
          //printf("novy hrac ale je plno\n");
          char * msg = "Plno";
          my_send(new_player->fd, msg);
       //   int conv_next_msg_size = htonl(strlen(msg));
       //   send(new_player->fd, &conv_next_msg_size, sizeof(conv_next_msg_size), 0);
       //   send(new_player->fd, msg, strlen(msg), 0);
          close(new_player->fd);
          continue;
        }
        //printf("Novy hrac %d\n", new_player->fd);
        struct ser_pla* sp = calloc(1, sizeof(ser_pla));//{new_player, this};
        sp->player = new_player; //uvolni sa v player_init_a_dispache
        sp->server = this;
        pthread_create(&new_player->thread, NULL, player_init_a_dispache, sp);
        
        
      }else
      {
        //printf("prisiel event ale nie POLL IN\n");
        continue;

      }
    }
  }
  return NULL;
}

void remove_player_from_players(void * data, void * in, void * out, void * err)
{
  int index = *(int *)data;
  //printf("removing player at %d\n", index);
  sll * players = in;
  sll_remove(players, index);
}

void stop_players(void * data, void * in, void * out, void * err)
{
  player * p = *(player **) data;
  p->action = P_GAME_END;
  p->work = 0;
  my_send(p->fd, "e-end");
  //destroy player joinduje thread
}
void * player_init_a_dispache(void * arg)
{
  struct ser_pla * sp = arg;
  struct server* server = sp->server;
  struct player* player = sp->player;

  free(sp);
  
  pthread_mutex_init(&player->mut_action, NULL);
  player->id = player->fd;
  //printf("new player %d created\n", player->id);

  //char * msg = "OK";
  my_send(player->fd, "OK");

  player->body=calloc(1, sizeof(struct sll));
  sll_init(player->body, sizeof(struct coord));
  

  pthread_mutex_lock(server->mut_map);
  coord newPos = get_coord_for_new_player(server->map);
  pthread_mutex_unlock(server->mut_map);

  sll_add(player->body, &newPos);

  int col = rand()%4;
  switch (col) {
    case 0:
      player->colour = RED_P_C;
    break;
    case 1:
      player->colour = GREEN_P_C;
    break;
    case 2:
      player->colour = WHITE_P_C;
    break;
    case 3:
      player->colour = BLUE_P_C; 
  }

//  int conv_next_msg_size = htonl(strlen(msg));
//  send(player->fd, &conv_next_msg_size, sizeof(conv_next_msg_size), 0);
//  send(player->fd, msg, strlen(msg), 0);

  player->action = newPos.x <= server->map->dim.x ? '>' : '<';
  player->next_action = newPos.x <= server->map->dim.x ? '>' : '<'; // netreba mutexovat lebo player_in_task este nebezi
  player->prev_direction = player->action;
  player->work = 1;

  pthread_mutex_lock(server->mut_players);
  sll_add(server->players, &player);
  time_t start_at = time(NULL)+3;
  sll_for_each(server->players, &set_players_start_at_time_to, &start_at, NULL,NULL);
  pthread_mutex_unlock(server->mut_players);

  player_in_task(player);
  return NULL;
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
  struct pollfd fds[0];

  time_t no_player_time_start = 0;
  time_t curr_time;
  while (this->work)
  {
    //printf("%d %d %d %d", this->time_duration, this->time_start, this->time_duration+this->time_start, time(NULL));
    if((this->time_duration > 0) && (this->time_start+this->time_duration <= time(NULL)))
    {
      printf("server konci\n");
      this->work = 0;
      sll_for_each(this->players, &stop_players, NULL, NULL, NULL);
    }
  
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
    //  printf("no players for %d s\n", (curr_time - no_player_time_start));
    }
    else
    {
      server_tick(this);
      if(no_player_time_start != 0)
      {
        no_player_time_start = 0;
      }
    }
 //   struct timespec ts = {SERVER_TICK/1000, (SERVER_TICK%1000)*1000000};
 //   nanosleep(SERVER_TICK);
    poll(fds,0,SERVER_TICK);
  }
  return NULL;
}

void server_tick(struct server * this)
{
  sll index_endedPlayers;
  sll_init(&index_endedPlayers, sizeof(int));
  int index = 0;

  //clone_map(this->map_no_players, this->map, this->max_map);
  pthread_mutex_lock(this->mut_map);

  reset_map(this->map);

  pthread_mutex_lock(this->mut_players);
  sll_for_each(this->players, &server_ack_player_next_action, &index, &index_endedPlayers, NULL);
  
  if(sll_get_size(&index_endedPlayers) > 0)
  {
    sll_for_each(&index_endedPlayers, &remove_player_from_players, this->players, NULL, NULL);
  }

  sll_for_each(this->players, &server_do_player_action, &this->fruit_left, this->map, NULL);
//  print_map(this->map->map, this->map->dim);
  

//  printf("left %d numPl %d \n",this->fruit_left, sll_get_size(this->players));
  if(this->fruit_left < sll_get_size(this->players))
  {
 //   printf("generating new fruit\n");
    if(try_generate_fruit(this->map)==1)
    {
      this->fruit_left++;
    }
  }

  char * serializedM;
  size_t aloc_map = serialize_map(this->map, &serializedM);
  char * serialized_P;
  serialize_players(this->players, time(NULL)-this->time_start, &serialized_P);
  //printf("%s\n",serialized_P);
  size_t len_of_pl = strlen(serialized_P);
  char* frame = realloc(serialized_P, aloc_map);
  sprintf(&frame[len_of_pl], "%s", serializedM);
  //printf("%s\n", frame);
 // sll_for_each(this->players, &players_check_colision_w_other_players, this->players, NULL, NULL);
  sll_for_each(this->players, &players_check_colision_w_other_players_and_send_map, this->players, this->map->map, frame);
  pthread_mutex_unlock(this->mut_players);

  pthread_mutex_unlock(this->mut_map);

  sll_clear(&index_endedPlayers);
  free(serializedM);
  free(frame);

  
}

void server_destroy(struct server * this)
{
  this->work = 0;
  sll_for_each(this->players, &destroy_player, NULL, NULL, NULL);
  pthread_mutex_destroy(this->mut_players);
  free(this->mut_players);

  pthread_mutex_destroy(this->mut_map);
  free(this->mut_map);

  sll_clear(this->players);
  free(this->players);

  if (this->map->dim.x != -1)
  { map_destroy(this->map); }
  free(this->map);

}
//--------------------------------------
//int main (int argc, char* argv[])
//{
//  struct server s;
//  struct run_param rp = {"127.0.0.1", 8080, -1, 5, "", 20, 20};
//  server_init(&s, &rp);
//  printf("server initialized\n");
//  server_start(&s);
//  printf("server ended\n");
//  server_destroy(&s);
//}

void server_dispache(run_param * rp)
{
  struct server s;
  if (server_init(&s, rp)!=1)
  {
    printf("\033[1;5;30;101mNepoderilo sa vytvorit server\033[0m\n");
    server_destroy(&s);
    return;
  }
  server_start(&s);
  server_destroy(&s);
}

