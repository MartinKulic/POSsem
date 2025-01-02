//server.h
#pragma once


#include <pthread.h>
#include <stdatomic.h>
#include <netinet/in.h>
#include <pthread.h>

#include "../libsll/sll.h"

#define PORT 8080
 

typedef struct player{
  int id;
  int fd;
  int scor;
  char action;
  char next_action;
  pthread_mutex_t mut_action;
  pthread_t thread;
  //colur
  struct sll * body;
}player;

typedef struct server {
  struct sll* players;
  size_t MAX_PLAYERS;
  int count_active_players;
  int server_fd;
  struct sockaddr_in address;
  atomic_bool work;
  pthread_mutex_t* mut_players;
} server;
typedef struct ser_pla{
  struct player * player;
  struct server * server;
} ser_pla;

int server_init(struct server* this, int port);
void server_start(struct server* this);
void * server_connect_players(void * arg);
void * player_init_a_dispache(void * arg);

void server_destroy(struct server * this);
