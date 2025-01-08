//player.h

#include <pthread.h>
#include <stdatomic.h>

#include "../libsll/sll.h"
#include "map.h"

#define TIME_UNPAUSE 3


typedef struct player{
  int id;
  int fd;
  int scor;
  char prev_direction;
  char action;
  char next_action;
  atomic_bool work;
  pthread_mutex_t mut_action;
  pthread_t thread;
  char * colour;
  struct sll * body;
  time_t start_move_at;
}player;

void player_in_task(struct player* this);
void set_players_start_at_time_to(void* data, void* in, void* out, void* err);
void server_ack_player_next_action(void * d, void * i, void * o, void * e);
void server_do_player_action(void * d, void * i, void * o, void * e);
void player_move(struct player* this, struct coord direction, map* map, int * fruit_left);
void player_dont_move(player * this, map * map);
void players_check_colision_w_other_players_and_send_map(void * d, void * i, void * o, void * e);
_Bool player_check_colision_w_other_players(struct player * this, sll * players); //Nepouzivasa

void destroy_player(void * data, void * in, void * out, void * err);

void serialize_players(sll * players, int serv_time, char ** dest);
