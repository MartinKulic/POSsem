//client.c

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main (int argc, char* argv[])
{
  int status, valread, client_fd;
  struct sockaddr_in serv_addr;
  //char buffer[1024] = {0};

  if((client_fd=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // convert add to bin
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
  {
    printf("Invalid address\n");
    return -1;
  }

  if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("Connection fail\n");
    return -1;
  }

  send(client_fd, "hello is tam", 13,0);

  for (int i = 0; i < 5; i++)
  {
    char* buffer[1024] = {0};
    valread = read(client_fd, buffer, 1024-1);
    printf("K-prijal: %s\n", buffer);
  }

  close(client_fd);

  return 0;
}
