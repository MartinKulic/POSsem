//server.c

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main (int argc, char *argv[])
{
  int server_fd, new_socket;
  ssize_t valred;
  struct sockaddr_in address;
  int opt =1;
  socklen_t addrlen = sizeof(address);
  char buffer[1024] = {0};
  
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

  if ( (new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0)
  {
    perror("accept failed");
    exit(EXIT_FAILURE);
  }
  valred = read(new_socket, buffer, 1024-1);
  printf("S-prijal: %s\n", buffer);

  for(int i = 0; i < 5; i++)
  {
    char serverMessage[100] = {0};
    scanf("%s", &serverMessage);
    send(new_socket, serverMessage, strlen(serverMessage), 0);
  }

  //close connected socket
  close(new_socket);
  //close listenong socket
  close(server_fd);

  return 0;
}
