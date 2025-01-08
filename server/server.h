//server.h
#pragma once


#include <pthread.h>
#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#include "../libsll/sll.h"
#include "../share/com_protocol.h"
#include "../share/run_param.h"
#include "map.h"
#include "player.h"

//#define PORT 8080
#define NO_ACTIVY_SERVER_END 10 //10 sekund
#define SERVER_TICK 250




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
  time_t time_start;
  time_t time_duration;
} server;

typedef struct ser_pla{
  struct player * player;
  struct server * server;
} ser_pla;

int server_init(struct server* this, run_param * rp);
void server_start(struct server* this);
void * server_connect_players(void * arg);
void * server_logic(void * arg);
void * player_init_a_dispache(void * arg);
void server_tick(struct server * this);

void remove_player_from_players(void* d, void * i, void * o, void *e);
void stop_players(void * data, void * in, void * out, void * err);

void server_destroy(struct server * this);

void server_dispache(run_param * rp);
