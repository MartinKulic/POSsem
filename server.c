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

typedef struct in_data
{
  int socket;
  atomic_bool work;
  char* input_buffer;
}in_data;

void * in_task(void * arg){
  in_data * this = arg;
  //while(this->work)
 // {
    char buffer[1024] = {0};
    ssize_t valred = read(this->socket, buffer, 1024-1);
    if (valred == 0){
      //break;
    }
    //printf("S-valred: %d\n", valred);

    *this->input_buffer = buffer[0];
    printf("V buffery: %c z %d\n", *this->input_buffer, this->socket);

  //}

  printf("%d ending, work = %d\n", this->socket, this->work);
  close(this->socket);
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
  //send(client_fd, "hello is tam", 13,0);
  int lst = 0; // conter for i_data array;
  //while(work)
  for (int i = 1; i<4; i++)
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

    i_data[lst] = newData;

    pthread_create(&threads[lst], NULL, in_task, newData);

    lst++;

    }

  

  for(int i = 0; i < lst; i++)
  {
    printf("joinding\n");
   // close(i_data[i]->socket);
   // printf("socket %d closed\n", i_data[i]->socket);
    //pthread_kill(threads[i], 1);
    pthread_join(threads[i], NULL);
    printf("joinded\n");
    free(i_data[i]);
  }
  for (int i = 0; i < lst; i++)
  {
  //  i_data[i]->work = 0;
    printf("%d - %c\n", i, inputs[i]);
  }

  free(threads);
  //close listenong socket
  close(server_fd);

  return 0;
}
