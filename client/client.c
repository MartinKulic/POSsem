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
#include <poll.h>

#include "client.h"

#define PORT 8080

// 1 ak prebehne uspesne 
int connect_to_server (int * client_fd)
{
//input_data* this = argv;
  int status;
  struct sockaddr_in serv_addr;
  //char buffer[1024] = {0};

  if((*client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Socket creation error \n");
    return 0;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // convert add to bin
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  {
    printf("Invalid address\n");
    return 0;
  }

  if ((status = connect(*client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("Connection fail\n");
    return 0;
  }

  printf("connected\n");
  return 1;
  
}
void * communication_task(void * arg)
{
  struct communication_data* this = arg;

  struct termios oldt, newt;
  tcgetattr(0, &oldt);
  memcpy(&newt, &oldt, sizeof(struct termios));
  newt.c_lflag &= ~(ICANON | ECHO);
  ////polling read
  //blocking read
  newt.c_cc[VTIME] = 0; // timeot in deciseconds for noncanonical read
  //newt.c_cc[VMIN] = 1;  // minimum number of cahracters for noncanocical read
  tcsetattr(0, TCSANOW, &newt);
  //cfmakeraw();

  this->fds[0].fd = STDIN_FILENO;
  this->fds[0].events = POLLIN;

  this->fds[1].fd = this->client_fd;
  this->fds[1].events = POLLIN;

  
  while(this->work)
  {
   // printf("enter command: ");
 
    printf("bfr poll\n");
    if (poll(this->fds,1,900)>0)
    {
      if (this->fds[0].revents & POLLIN)
      {
        com_in_task(this);
      }// je 1. char na citanie v fd
      if (this->fds[0].revents & POLLIN)
      {
        com_out_task(this);
      }

    }// je 0 fd je ready
    
    
  }
  tcsetattr(0, TCSANOW, &oldt); 
  close(this->client_fd);
  return NULL;
}

//Recievs data from server responds to them if needed
void com_out_task(struct communication_data* data)
{
  ;
}

//Manages input from user end send them to server
void com_in_task(struct communication_data* data)
{
  char ch[3];
  
  read(data->fds[0].fd, &ch[0], 1);
  printf("%d   ", ch[0]);
  if(ch[0]=='\033')
  {
    if(poll(data->fds, 1, 5)>0)
    {
      read(data->fds[0].fd, &ch[1], 1);
      if(ch[1]=='[')
      {
        char toSend[1];
        if(poll(data->fds, 1, 5)>0)
        {
          read(data->fds[0].fd, &ch[2], 1);
          switch(ch[2])
          {
            case 'A':
              toSend[0] = 'A';
              break;
            case 'B':
              toSend[0] = 'V';
              break;
            case 'C':
              toSend[0]= '>';
              break;
            case 'D':
              toSend[0]='<';
              break;
          }// 3. char
          printf("%c\n", toSend[0]);
          send(data->client_fd, toSend, 1, 0);
        }// je 3. char
      }// 2. char je [
    }// je 2. char
    else // nie je 2. char
    {
      printf("esc\n");
      send(data->client_fd, "q", 1, 0);
      data->work = 0;
    }
  }// 1. char je 27 - \033
}

//--------------------------------------------

int main (int argc, char* argv[])
{
  int client_fd = 0;
  if (connect_to_server(&client_fd) != 1)
  {
    return 2;
  }
  struct communication_data * cd = calloc(1, sizeof(struct communication_data));
  cd->work = 1;
  cd->client_fd = client_fd;
  communication_task(cd);  

  free(cd);
  return 0;
}
