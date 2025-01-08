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
#include "../share/com_protocol.h"

#define PORT 8080


void client_init(communication_data * this)
{
  this->client_fd = -1;

  this->fds[0].fd = STDIN_FILENO;
  this->fds[0].events = POLLIN;

  //this->fds[1].fd = this->client_fd; //client_fd will get walue after connect_to_server
  this->fds[1].events = POLLIN;
}

// 1 ak prebehne uspesne 
int connect_to_server (int * client_fd, run_param * rp)
{
//input_data* this = argv;
  int status;
  struct sockaddr_in serv_addr;
  //char buffer[1024] = {0};

  if((*client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\033[31mSocket creation error\033[0m \n");
    return 0;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(rp->port);

  // convert add to bin
  if(inet_pton(AF_INET, rp->ip, &serv_addr.sin_addr)<=0)
  {
    printf("\033[31mInvalid address\033[0m\n");
    return 0;
  }

  if ((status = connect(*client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("\033[1;31mConnection fail\033[0m\n");
    return 0;
  }
  printf("\033[1;4;32mConnected\033[0m\n", *client_fd, status);
  char buff[STD_TRANSFER_LEN] = {0};
  my_recv(*client_fd, &buff[0]);
  printf("%s\n", buff);

  if(strcmp("Plno", buff)==0){
    printf("\033[31mServer je plny\033[0m\n");
    return 0;
  }
 
  /*int ident_but_net = htonl(*uniqe_identifier);
  send(*client_fd,(char*)&ident_but_net, 4, 0);
  
  recv(*client_fd, &ident_but_net, 4, 0);
  *uniqe_identifier = ntohl(ident_but_net);
  printf(" uniqe_identifier: %c %d\n", *uniqe_identifier, *uniqe_identifier);*/
  return 1;
  
}
void * communication_task(void * arg)
{
  struct communication_data* this = arg;

  this->fds[1].fd = this->client_fd;

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

  send(this->client_fd, "u", 1, 0);
  
  while(this->work == 1)
  {
   // printf("enter command: ");
    if (poll(this->fds,2,900)>0)
    {
      if (this->fds[0].revents & POLLIN)
      {
        com_out_task(this);
      }// je 1. char na citanie v fd
      if (this->fds[1].revents & POLLIN)
      {
        com_in_task(this);
      }// prislo nieco zo servera
      
    }// je 0 fd je ready 
  }
  tcsetattr(0, TCSANOW, &oldt);
  return NULL;
}

//Recievs data from server responds to them if needed
void com_in_task(struct communication_data* data)
{
  char * msg;
  //printf("incoming t: ");
  my_recv_large(data->client_fd, &msg);
  switch(msg[0]){
    case T_FRAME:
      printf("\e[1;1H\e[2J%s\n", &msg[1]);
    break;
    case 'e':
      data->work = 2;
      printf("end %s %d\n", msg, data->work);
    break;
  }

  free(msg);
}

//Manages input from user end send them to server
void com_out_task(struct communication_data* data)
{
  char ch[3];
  
  read(data->fds[0].fd, &ch[0], 1);
 // printf("%d   ", ch[0]);
  if(ch[0]=='\033')
  {
    if(poll(data->fds, 1, 5)>0 && (data->fds[0].revents & POLLIN))
    {
      read(data->fds[0].fd, &ch[1], 1);
      if(ch[1]=='[')
      {
        char toSend[1];
        if(poll(data->fds, 1, 5)>0 && (data->fds[0].revents & POLLIN))
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
    else// nie je 2. char
    {
      printf("esc\n");
      send(data->client_fd, "p", 1, 0);
      data->work = 0;
    }
  }// 1. char je 27 - \033
 
}


void client_destroy(communication_data * this)
{
  if(this->client_fd > 0)
  {
    if (this->work !=2)
    {
      send(this->client_fd, "q", 1, 0);
    }
    close(this->client_fd);
  }
  free(this);
}

//--------------------------------------------

//int main (int argc, char* argv[])
//{
//  struct communication_data * cd = calloc(1, sizeof(struct communication_data));
// // cd->uniqe_identifier = 0;
//
//  run_param rp = {"127.0.0.1", 8080, -1, 5, "", 20, 20};
//
//  printf("odpojeny e exit other reconect\n");
//  while(getchar()!='e')
//  {
//    if (connect_to_server(&cd->client_fd, &rp) != 1)
//    {
//      return 2;
//    }
//    cd->work = 1;
//    communication_task(cd); 
//    printf("odpojeny esc exit other reconect\n");
//  }
//  free(cd);
//  return 0;
//}

void client_dispache(run_param * rp)
{
  communication_data * this = calloc(1, sizeof(communication_data));
  client_init(this);
  if (connect_to_server(&this->client_fd, rp) != 1)
  {
    printf("\033[1;5;30;41mNepodarilo sa pripojit na server!\033[0m\n\033[3;37mUisti sa, ze adresa a port su spravne.\033[0m\n");
    this->client_fd = -1;
    client_destroy(this);
    return ;
  }
  this->work = 1;
  communication_task(this);
  client_destroy(this);
}
