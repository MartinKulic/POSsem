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

#define PORT 8080

typedef struct input_data{

}input_data;
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
void * communication_task(int client_fd)
{
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

  struct pollfd fds[1];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  char ch[3];
  while(1)
  {
   // printf("enter command: ");
 
    printf("bfr poll\n");
    if (poll(fds,1,900)>0)
    {
      if (fds[0].revents & POLLIN)
      {
        read(fds[0].fd, &ch[0], 1);
        printf("%d   ", ch[0]);
        if(ch[0]=='\033')
        {
          if(poll(fds, 1, 5)>0)
          {
            read(fds[0].fd, &ch[1], 1);
            if(ch[1]=='[')
            {
              char toSend[1];
              if(poll(fds, 1, 5)>0)
              {
                read(fds[0].fd, &ch[2], 1);
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
                send(client_fd, toSend, 1, 0);
              }// je 3. char
            }// 2. char je [
          }// je 2. char
          else // nie je 2. char
          {
            printf("esc\n");
            send(client_fd, "q", 1, 0);
            break;
          }
        }// 1. char je 27 - \033
      }// je 1. char na citanie v fd
    }// ne 0 fd je ready
    
    
  }
  tcsetattr(0, TCSANOW, &oldt); 
  close(client_fd);
  return NULL;
}

int main (int argc, char* argv[])
{
  int client_fd = 0;
  if (connect_to_server(&client_fd) != 1)
  {
    return 2;
  }
  communication_task(client_fd);  

  return 0;
}
