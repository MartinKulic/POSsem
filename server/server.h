//server.h
#pragma once


#include <pthread.h>
#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#include "../libsll/sll.h"
#include "../share/com_protocol.h"

#define PORT 8080
#define NO_ACTIVY_SERVER_END 10 //10 sekund 

typedef struct coord{
  int x;
  int y;
}coord;
typedef struct player{
  int id;
  int fd;
  int scor;
  char action;
  char next_action;
  atomic_bool work;
  pthread_mutex_t mut_action;
  pthread_t thread;
  //colur
  struct sll * body;
}player;

typedef struct server {
  struct sll* players;
  size_t MAX_PLAYERS;
  int server_fd;
  struct sockaddr_in address;
  atomic_bool work;
  pthread_mutex_t* mut_players;
  char ** map;
  char ** no_player_map;
  struct coord MAX_MAP;
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
void player_in_task(struct player* this);
void server_tick(struct server * this);
void server_ack_player_next_action(void * d, void * i, void * o, void * e);
void server_do_player_action(void * d, void * i, void * o, void * e);

void clone_map(char** src, char** dest, coord dim);
void player_move(struct player* this, struct coord direction, char ** map);
struct coord generate_fruit(char ** map);

void remove_player_from_players(void* d, void * i, void * o, void *e);

void print_map(char ** map, struct coord dim);

void destroy_player(void * data, void * in, void * out, void * err);
void server_destroy(struct server * this);
