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

typedef struct input_data{

}input_data;

void * connect_to_server (int * client_fd)
{
//input_data* this = argv;

  int status;
  struct sockaddr_in serv_addr;
  //char buffer[1024] = {0};

  if((*client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

  if ((status = connect(*client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("Connection fail\n");
    return NULL;
  }

  printf("connected\n");

  
}
void * input_task(int client_fd)
{
  struct termios oldt, newt;
  tcgetattr(0, &oldt);
  memcpy(&newt, &oldt, sizeof(struct termios));
  newt.c_lflag &= ~(ICANON | ECHO);
  ////polling read
  //blocking read
  newt.c_cc[VTIME] = 0; // timeot in deciseconds for noncanonical read
 // newt.c_cc[VMIN] = 1;  // minimum number of cahracters for noncanocical read
  tcsetattr(0, TCSANOW, &newt);
  //cfmakeraw();

  char ch[3];
  while(1)
  {
   // printf("enter command: ");
    
    read(STDIN_FILENO, &ch[0], 1);
    if(ch[0] == '\033')
    {
      read(STDIN_FILENO, &ch[1], 1);
      if(ch[1]=='[')
      {
        read(STDIN_FILENO, &ch[2], 1);
        if (ch[2]!=0)
        {
          char toSend[1]={0};
          switch(ch[2])
          {
            case 'A':
              toSend[0] = 94;
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
          }
          printf("%c\n", toSend[0]);
          send(client_fd, toSend, 1, 0);
        }
        
      } else
        {
          printf("esc\n");
          break;
        }
    } if (ch[0]==27){
      printf("esc\n");
      break;
    }
  }
  tcsetattr(0, TCSANOW, &oldt); 
  close(client_fd);
  return NULL;
}

int main (int argc, char* argv[])
{
  int client_fd = 0;
  connect_to_server(&client_fd);
  input_task(client_fd);  

  return 0;
}
