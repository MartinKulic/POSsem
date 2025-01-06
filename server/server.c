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


int server_init(struct server* this, int port)
{
  // setup communication
  int opt = 1;
  this->MAX_PLAYERS = 5;
  this->MAX_MAP = (struct coord){50,25};
  
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
  
  this->map = malloc(this->MAX_MAP.y * sizeof(map_cell*));//riadky
  this->no_player_map = malloc(this->MAX_MAP.y * sizeof(map_cell*));
  for(size_t i = 0; i < this->MAX_MAP.y; i++)
  {
    this->map[i] = malloc(this->MAX_MAP.x * sizeof(map_cell));//znaky v riadkoch

    this->no_player_map[i] = malloc(this->MAX_MAP.x * sizeof(map_cell));
    
    for(size_t ii = 0; ii < this->MAX_MAP.x; ii++)
    {
      this->map[i][ii] = (struct map_cell){'-', RESET};
      this->no_player_map[i][ii] = EMPTY_CELL;
    }
    //printf("%c\n", this->map[i][2]);
  }

  this->fruit_left=0;
  printf("mapa:\n");
  print_map(this->map, this->MAX_MAP);
  printf("no_player_map: \n");
  print_map(this->no_player_map, this->MAX_MAP);
 
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
  return NULL;
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

  player->body=calloc(1, sizeof(struct sll));
  sll_init(player->body, sizeof(struct coord));
  
  struct coord init = {2,2};
  sll_add(player->body, &init);

  player->colour = BLUE_P_C;

//  int conv_next_msg_size = htonl(strlen(msg));
//  send(player->fd, &conv_next_msg_size, sizeof(conv_next_msg_size), 0);
//  send(player->fd, msg, strlen(msg), 0);
  player->work = 1;

  player_in_task(player);
  return NULL;
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
      //printf("player %d revent %d ", this->id, fds[0].revents);
      if(fds[0].revents & POLLIN)
      {
        char n_a[1];
        recv(this->fd, &n_a, 1, 0);

        pthread_mutex_lock(&this->mut_action);
        this->next_action = *n_a;
        pthread_mutex_unlock(&this->mut_action);

        
        //printf("player %d recieved %c\n", this->id, this->next_action);
        if(this->next_action == 'q')
        {
          this->work = 0;
          break;
        }

  //      my_send(this->fd, "OK");
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
  
  sll_clear(this->body);
  free(this->body);

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
  struct pollfd fds[0];

  time_t no_player_time_start = 0;
  time_t curr_time;
  while (this->work)
  {
  
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

  clone_map(this->no_player_map, this->map, this->MAX_MAP);

  pthread_mutex_lock(this->mut_players);
  sll_for_each(this->players, &server_ack_player_next_action, &index, &index_endedPlayers, NULL);
  
  if(sll_get_size(&index_endedPlayers) > 0)
  {
    sll_for_each(&index_endedPlayers, &remove_player_from_players, this->players, NULL, NULL);
  }
  pthread_mutex_unlock(this->mut_players);

    sll_for_each(this->players, &server_do_player_action, &this->fruit_left, this->map, this->no_player_map);
  print_map(this->map, this->MAX_MAP);

  printf("left %d numPl %d \n",this->fruit_left, sll_get_size(this->players));
  if(this->fruit_left < sll_get_size(this->players))
  {
    printf("generating new fruit\n");
    if(try_generate_fruit(this->map, this->no_player_map, this->MAX_MAP)==1)
    {
      this->fruit_left++;
    }
  }

  sll_for_each(this->players, &players_check_colision_w_other_players, this->players, NULL, NULL);

  sll_clear(&index_endedPlayers);

  
}

void server_ack_player_next_action(void * data, void * in, void * out, void * err)
{
  struct player * player = *(struct player **) data;
  int * index = (int *)in;
  //printf("\tplayer-%d-i>%d-na> %c -a> %c\n", player->id, *index, player->next_action, player->action);
  switch (player->next_action){
    case P_GAME_QUIT:
      destroy_player(&player,NULL,NULL,NULL);
      sll * index_p = out;
      sll_add(index_p, index);
      *index = *index+1;
      return ;
    break;
    case P_GAME_PAUSE:

    break;
    case P_GAME_END:
      *index = *index+1;
      return ;
    break;
  }
  if(player->action == P_GAME_END)
  {
    *index = *index+1;
    return ;
  }

  if(((player->action=='<' || player->action=='>') && (player->next_action=='>'||player->next_action=='<'))
    || ((player->action=='A' || player->action=='V') && (player->next_action=='V' || player->next_action=='A')) )
  {
    pthread_mutex_lock(&player->mut_action);
    player->next_action = player->action;
    pthread_mutex_unlock(&player->mut_action);
  }

  *index = *index+1;
  pthread_mutex_lock(&player->mut_action);
 
  player->action = player->next_action;
  
  pthread_mutex_unlock(&player->mut_action);
   
}

void server_do_player_action(void * data, void * in, void * out, void * err)
{
  struct player * player = *(struct player **) data;
  map_cell ** map = out;
  map_cell ** map_n_p = err;
  int * fruit_left = (int *) in;

  printf("plr action %c\n", player->action);
    
  switch (player->action)
  {
    case 'A':
      player_move(player, (struct coord){0, (int)(-1)}, map, map_n_p, fruit_left);
    break;
    case 'V':
      player_move(player, (struct coord){0, 1}, map, map_n_p, fruit_left);
    break;
    case '<':
      player_move(player, (struct coord){-1, 0}, map, map_n_p, fruit_left);
    break;
    case '>':
      player_move(player, (struct coord){1, 0}, map, map_n_p, fruit_left);
    break;
    case P_GAME_PAUSE:
      player_move(player, (struct coord){0, 0}, map, map_n_p, fruit_left);
    break;
  }
  
}

void player_move(struct player* this, struct coord direction, map_cell ** map, map_cell** map_n_p, int * fruit_left)
{
  struct coord * headData = (struct coord*)this->body->head_->data_;
  struct coord prev_position;
  prev_position = *headData;
  headData->x += direction.x;
  headData->y += direction.y;
  printf("p %d ( %d ; %d )", this->id, headData->x, headData->y);

  
  if (map[headData->y][headData->x].ch == FRUIT_CH)
  {
    sll_add(this->body, &prev_position);
    (*fruit_left)--;
    this->scor++;
    map_n_p[headData->y][headData->x] = EMPTY_CELL;
  }
  else if (map[headData->y][headData->x].ch != MAP_EMPTY)//do prostredia pripadne do inych hadov
  {
    this->action = P_GAME_END;
  }

  map[headData->y][headData->x] = (struct map_cell){this->action,this->colour};

  sll_node* node = this->body->head_->next_;
  while(node != NULL)
	{
		struct coord helper = *(struct coord*)node->data_;
    *(struct coord*)node->data_ = prev_position;
    prev_position = helper;

    map[((struct coord*)(node->data_))->y][((struct coord*)(node->data_))->x] = (struct map_cell){'H',this->colour};
    printf(" -> ( %d ; %d )", ((struct coord*)(node->data_))->x, ((struct coord*)(node->data_))->y);

		node = node->next_;
	}

  _Bool re =check_colision(map, *(struct coord*)this->body->head_->data_, this->action);
  printf("res %d,\n",re);
  if(re==1)//sam do seba pripadne do inych hadov
  {
    this->action = P_GAME_END;
  }

	printf("\n");

}
void players_check_colision_w_other_players(void * data, void * in, void * out, void * err)
{
  player * player = *(struct player**)data;
  sll * players = in;
 
  if(player->action != P_GAME_PAUSE)
  {
    if(player_check_colision_w_other_players(player, players) == 1)
    {
      player->action = P_GAME_END;
    }
  }

}

_Bool player_check_colision_w_other_players(player * this, sll * players)
{
  coord* my_head_coord = this->body->head_->data_;
  sll_node* players_node = players->head_;//prvy hrac
	while(players_node != NULL)
	{
	  player * other_player = *(player**)players_node->data_;
	  if(other_player != this)
	  {
      sll_node* other_body_node = other_player->body->head_;//ptva cast tela ineho hraca;
      while (other_player != NULL)
      {
        coord* other_body_coord = (coord *)other_body_node->data_;
        if(other_body_coord->x==my_head_coord->x && other_body_coord->y==my_head_coord->y)
        {
          return 1;
        }
        other_body_node = other_body_node->next_;
      }
	  }
    players_node = players_node->next_;
	}
  return 0;
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

  for(size_t i = 0; i < this->MAX_MAP.y; i++)
  {
    free(this->map[i]);
    free(this->no_player_map[i]);
  }
  free(this->map);
  free(this->no_player_map);
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
