//player.c
#include <sys/socket.h>
#include <poll.h>
#include <sys/poll.h>
#include <unistd.h>

#include "player.h"
#include "../share/com_protocol.h"


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
        this->next_action = *n_a == 0 ? P_GAME_QUIT : *n_a; // ak primeme 0 najskor doslo k chybe

        //printf("player %d recieved %c\n", this->id, this->next_action);
        if(this->next_action == P_GAME_QUIT)
        {
          pthread_mutex_unlock(&this->mut_action);
          this->work = 0;
          break;
        }
        pthread_mutex_unlock(&this->mut_action);
  //      my_send(this->fd, "OK");
      }
    }
  }
}

void set_players_start_at_time_to(void* data, void* in, void* out, void* err)
{
  player * p = *(player**)data;
  time_t * st = in;
  p->start_move_at = *st+TIME_UNPAUSE;
}

void server_ack_player_next_action(void * data, void * in, void * out, void * err)
{
  struct player * player = *(struct player **) data;
  int * index = (int *)in;
  
  //printf("\tplayer-%d-i>%d-na> %c -a> %c -pd> %c \n", player->id, *index, player->next_action, player->action, player->prev_direction);
 
  pthread_mutex_lock(&player->mut_action);
  switch (player->next_action){
    case P_GAME_QUIT:
      destroy_player(&player,NULL,NULL,NULL);
      sll * index_p = out;
      sll_add(index_p, index);
      *index = *index+1;
      return ;
    break;
    case P_GAME_PAUSE:
      if(player->action != P_GAME_PAUSE){
      player->prev_direction = player->action;
      }
    break;
    case P_GAME_UPAUSE:
      player->next_action = player->prev_direction;
    break;
    case P_GAME_END:
      *index = *index+1;
      return ;
    break;
  }

  switch(player->action)
  {
    case P_GAME_END:
      *index = *index+1;
     // player->next_action = P_GAME_QUIT;
     // my_send(player->fd, "e-end");
      pthread_mutex_unlock(&player->mut_action);
      return ;
    break;
    case P_GAME_PAUSE:
      if(player->next_action != P_GAME_PAUSE){
        player->start_move_at = time(NULL)+3;
      }
  }
  pthread_mutex_unlock(&player->mut_action);

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
  map * map = out;
  int * fruit_left = (int *) in;

 // printf("plr action %c\n", player->action);

  //printf("%d %d\n", player->start_move_at, time(NULL));
  if(player->start_move_at > time(NULL))
  {  
    player_dont_move(player, map);
    return ;
  }
    
  switch (player->action)
  {
    case 'A':
      player_move(player, (struct coord){0, (int)(-1)}, map, fruit_left);
    break;
    case 'V':
      player_move(player, (struct coord){0, 1}, map, fruit_left);
    break;
    case '<':
      player_move(player, (struct coord){-1, 0}, map, fruit_left);
    break;
    case '>':
      player_move(player, (struct coord){1, 0}, map, fruit_left);
    break;
    case P_GAME_PAUSE:
      player_dont_move(player, map);
    break;
  }
  
}

void player_move(struct player* this, struct coord direction, map* map, int * fruit_left)
{
  struct coord * headData = (struct coord*)this->body->head_->data_;
  struct coord prev_position;
  prev_position = *headData;
  int new_x = (headData->x + direction.x);
  int new_y = (headData->y + direction.y);

  headData->x = new_x < 0 ? map->dim.x-1 : new_x % map->dim.x;
  headData->y = new_y < 0 ? map->dim.y-1 : new_y % map->dim.y;

  //printf("p %d ( %d ; %d )", this->id, headData->x, headData->y);

  
  if (map->map[headData->y][headData->x].ch == FRUIT_CH)
  {
    sll_add(this->body, &prev_position);
    (*fruit_left)--;
    this->scor++;
    map->map_no_players[headData->y][headData->x] = EMPTY_CELL;
  }
  else if (map->map[headData->y][headData->x].ch != MAP_EMPTY)//do prostredia pripadne do inych hadov
  {
    this->action = P_GAME_END;
  }

  map->map[headData->y][headData->x] = (struct map_cell){this->action,this->colour};

  sll_node* node = this->body->head_->next_;
  while(node != NULL)
	{
		struct coord helper = *(struct coord*)node->data_;
    *(struct coord*)node->data_ = prev_position;
    prev_position = helper;

    map->map[((struct coord*)(node->data_))->y][((struct coord*)(node->data_))->x] = (struct map_cell){SNAKE_BODY_CH,this->colour};
   // printf(" -> ( %d ; %d )", ((struct coord*)(node->data_))->x, ((struct coord*)(node->data_))->y);

		node = node->next_;
	}

  _Bool re =check_colision(map->map, *(struct coord*)this->body->head_->data_, this->action);
  //printf("res %d,\n",re);
  if(re==1)//sam do seba pripadne do inych hadov
  {
    this->action = P_GAME_END;
  }
}

void player_dont_move(player * this, map * map)
{
  //printf("p dont move\n");
  sll_node* node = this->body->head_;
  map->map[((struct coord*)(node->data_))->y][((struct coord*)(node->data_))->x] = (struct map_cell){this->action,this->colour};
  node = node->next_;
  while(node != NULL)
	{
    map->map[((struct coord*)(node->data_))->y][((struct coord*)(node->data_))->x] = (struct map_cell){SNAKE_BODY_CH,this->colour};
   // printf(" -> ( %d ; %d )", ((struct coord*)(node->data_))->x, ((struct coord*)(node->data_))->y);

		node = node->next_;
	}

}

void players_check_colision_w_other_players_and_send_map(void * data, void * in, void * out, void * err)
{
  player * player = *(struct player**)data;
  sll * players = in;
  map_cell** map = out;
  char * serialized_frame = err;

  if(player->action != P_GAME_PAUSE)
  {
    if(check_colision(map, *(coord*)player->body->head_->data_, player->action)==1)
    {
      player->action = P_GAME_END;
    }
  }

  my_send(player->fd, serialized_frame);
 
//  if(player->action != P_GAME_PAUSE)
//  {
//    if(player_check_colision_w_other_players(player, players) == 1)
//    {
//      player->action = P_GAME_END;
//    }
//  }

}

// Nepouziva sa
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



void serialize_players(sll * players, int serv_time, char ** destination)
{
  size_t index = 1;
  size_t pomLen = 0;
  char* t1 = "┢━━━━━━━━━━━━━━╍┅┉┈\n";
  size_t t1_size = strlen(t1);
  char* t2 = "┣━━━━━━━━━━━━━━╍┅┉┈\n";
  char* t3 = "┗━━━━━━━━━━━━━━╍┅┉┈\n";
  char* dest = malloc((50*sll_get_size(players)+(t1_size*3))+100);
  dest[0] = 'f';
  
  pomLen = sprintf(&dest[index], "┊  %d\n%s", serv_time, t1);
  index += pomLen;

  sll_node * node = players->head_;

  if( node != NULL )
  {
    
    while(node != NULL)
    {
      player * p = *(player**)node->data_;
      pomLen = sprintf(&dest[index], "┃%sH:>\033[0m  %d%s\n%s", p->colour, p->scor,p->action==P_GAME_END ? "  \033[1;91mX\033[0m" : " ", t2);
      index += pomLen;
      node = node->next_;
    }
  }

  sprintf(&dest[index], "%s\n", t3);
  *destination = dest;
}

void destroy_player(void * data, void * in, void * out, void * err)
{ 
  struct player * this = *(struct player **)data;
  //printf("destroy player %d a %p\n", this->fd, this);   
  //printf("destroing player %d", this->id);
  this->work = 0;
  pthread_join(this->thread, NULL);
  pthread_mutex_destroy(&this->mut_action);
  
  sll_clear(this->body);
  free(this->body);

  close(this->fd);

  free(this);
}
