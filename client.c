//client.c

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PORT 8080

typedef struct comunication_data{
  pthread_cond_t* cond;
  _Bool* wait;
  pthread_mutex_t* lock;
}comunication_data;

void * comunication_task(void* argv)
{
  comunication_data* this = argv;

  int status, valread, client_fd;
  struct sockaddr_in serv_addr;
  //char buffer[1024] = {0};

  if((client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Socket creation error \n");
    return NULL;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // convert add to bin
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  {
    printf("Invalid address\n");
    return NULL;
  }

  if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("Connection fail\n");
    return NULL;
  }

printf("connected\n");

  while (*this->wait) {
   // pthread_cond_wait(this->cond, this->lock);
 // printf("waiting\n");
  }

  send(client_fd, "s", 1, 0);

//  for (int i = 0; i < 5; i++)
//  {
//    printf("enter command: ");
//    char buffer[100] = {0};
//    scanf("%s", buffer);
//    send(client_fd, &buffer[0], 1, 0);
//  }
  time_t now = time(0);
  printf("%d\n", now);
  close(client_fd);
  return NULL;
}

int main (int argc, char* argv[])
{
  _Bool wait = 1;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  comunication_data com_data = {&cond, &wait, &lock};

  pthread_t * threads = calloc(5, sizeof(pthread_t));
  

  for (int i = 0; i<3; i++)
  { 
    pthread_create(&threads[i], NULL, comunication_task, &com_data);
  }

  sleep(1);
  printf("unlocking\n");
  wait = 0;
  //pthread_cond_broadcast(&cond);

  for (int i = 0;i<3;i++)
  {
    pthread_join(threads[i], NULL);
  }
  free(threads);
  return 0;
}
