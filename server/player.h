//player.h

#include <pthread.h>
#include <stdatomic.h>

#include "../libsll/sll.h"

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

void serialize_players(sll * players, int serv_time, char ** dest);
