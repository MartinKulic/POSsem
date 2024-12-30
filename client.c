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
#include <termios.h>

#define PORT 8080

typedef struct comunication_data{

}comunication_data;

void * comunication_task(void* argv)
{
  //comunication_data* this = argv;

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


  for (int i = 0; i < 5; i++)
  {
    printf("enter command: ");




    char buffer[100] = {0};
    scanf("%s", buffer);
    send(client_fd, &buffer[0], 1, 0);
  }
  close(client_fd);
  return NULL;
}

int main (int argc, char* argv[])
{
  comunication_task(NULL);  

  return 0;
}
