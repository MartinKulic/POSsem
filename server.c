//server.c

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <signal.h>
#include "server.h"

#define MAX_PLAYERS 5

typedef struct in_data
{
  int socket;
  atomic_bool work;
  char* input_buffer;
  pthread_mutex_t* mut;
}in_data;
void in_task_init(in_data* this, int socket, char* buff)
{
  this->socket = socket;
  this->work = 1; 
  this->input_buffer = buff;
}
void * in_task(void * arg){
  in_data * this = arg;
  while(this->work)
  //for (int i = 0; i<5; i++) // yatial ma hrac 5 tahov
  {
    char buffer[1024] = {0};
    ssize_t valred = read(this->socket, buffer, 1024-1);
    if (valred == 0){
      break;
    }
    pthread_mutex_lock(this->mut);
    *this->input_buffer = buffer[0];
    pthread_mutex_unlock(this->mut);

    printf("V buffery: %c z %d\n", *this->input_buffer, this->socket);

  }

  printf("%d ending, work = %d\n", this->socket, this->work);
  close(this->socket);
}
typedef struct logic_data{
  int* player_count;
  char * input_buffer;
  atomic_bool work;
  pthread_mutex_t * mutexes;
}logic_data;

void * logic_task(void * arg)
{
  logic_data * this = arg;
  char cpBuffer[MAX_PLAYERS];

  while((this->work)==1)
  {
    for(int i=0; i<*this->player_count;i++)
    {
      pthread_mutex_lock(&this->mutexes[i]);
      cpBuffer[i] = this->input_buffer[i];
      this->input_buffer[i] = 'n';
      pthread_mutex_unlock(&this->mutexes[i]);

      char player_command = cpBuffer[i];
      printf("player %d - %d = %c\n", i, player_command, player_command);

      //manage plater logic accorfingly
    }

    sleep(1);
  }
}
//---------------------------------------

int main (int argc, char *argv[])
{
  int server_fd, new_socket;
  ssize_t valred;
  struct sockaddr_in address;
  int opt =1;
  socklen_t addrlen = sizeof(address);
    
  //socket file descriptor
  if ( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  //forcefully attaching socket to the port PORT
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0 )
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // puts socket into passive mode
  if (listen(server_fd, 5) < 0)
  {
    perror("listen faile");
    exit(EXIT_FAILURE);
  }

  pthread_t * threads = calloc(5, sizeof(pthread_t));
  char inputs[5] = {'p'};
  in_data * i_data[5];
  int lst = 0; // conter for i_data array;
  pthread_mutex_t mutexes[MAX_PLAYERS] = {PTHREAD_MUTEX_INITIALIZER};

  logic_data ld = {&lst, &inputs[0], 1, &mutexes[0]};
  pthread_t t_logic;
  pthread_create(&t_logic, NULL, logic_task, &ld);

  for (int i = 1; i<MAX_PLAYERS; i++)
  {
    int new_socket;
    if ( (new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0)
    {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }
    printf("new connectien %d\n", new_socket);
  
    // new thread for player
    in_data* newData = calloc(1, sizeof(in_data));
    newData->socket = new_socket;
    newData->work = 1;
    newData->input_buffer = &inputs[lst];
    newData->mut = &mutexes[i];

    i_data[lst] = newData;

    pthread_create(&threads[lst], NULL, in_task, newData);

    lst++;
  }

  printf("press ENTER to kill remaining reading threads and continue\n");
  getchar();

  ld.work = 0;
  pthread_join(t_logic,NULL);
  printf("logic tread joinded\n");
  for(int i = 0; i < lst; i++)
  {
   // printf("joinding\n");
   // close(i_data[i]->socket);
   // printf("socket %d closed\n", i_data[i]->socket);
    //pthread_kill(threads[i], 1);
    pthread_kill(threads[i],1);
    printf("joinded\n");
    free(i_data[i]);
  }
  

  free(threads);
  //close listenong socket
  close(server_fd);

  return 0;
}
