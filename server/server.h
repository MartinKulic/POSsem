//server.h
#pragma once


#include <pthread.h>
#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#include "../libsll/sll.h"
#include "../share/com_protocol.h"
#include "map.h"

#define PORT 8080
#define NO_ACTIVY_SERVER_END 10 //10 sekund
#define SERVER_TICK 250

#define P_GAME_END 'e'
#define P_GAME_QUIT 'q'
#define P_GAME_PAUSE 'p'
#define P_NEW_PLAYER 'n'

typedef struct player{
  int id;
  int fd;
  int scor;
  char action;
  char next_action;
  atomic_bool work;
  pthread_mutex_t mut_action;
  pthread_t thread;
  char * colour;
  struct sll * body;
  time_t start_move_at;
}player;

typedef struct server {
  struct sll* players;
  size_t MAX_PLAYERS;
  int server_fd;
  struct sockaddr_in address;
  atomic_bool work;
  pthread_mutex_t* mut_players;
  pthread_mutex_t* mut_map;
  struct map * map;
  int fruit_left;
} server;

typedef struct ser_pla{
  struct player * player;
  struct server * server;
} ser_pla;

int server_init(struct server* this, int port);
void server_start(struct server* this);
void * server_connect_players(void * arg);
void * server_logic(void * arg);
void * player_init_a_dispache(void * arg);
void players_check_colision_w_other_players(void * d, void * i, void * o, void * e);
_Bool player_check_colision_w_other_players(struct player * this, sll * players);
void player_move(struct player* this, struct coord direction, map* map, int * fruit_left);
void set_players_start_at_time_to(void* data, void* in, void* out, void* err);
void player_in_task(struct player* this);
void server_tick(struct server * this);
void server_ack_player_next_action(void * d, void * i, void * o, void * e);
void server_do_player_action(void * d, void * i, void * o, void * e);

void remove_player_from_players(void* d, void * i, void * o, void *e);

void destroy_player(void * data, void * in, void * out, void * err);
void server_destroy(struct server * this);
