//comm_protocol.c
#include <arpa/inet.h>
#include <stdlib.h>

#include <stdio.h>

#include "com_protocol.h"


int my_send(int fd, char* targ)
{
  uint32_t sizeOfTransfer = strlen(targ)+1;
//  uint32_t sizeOfTransfer = 0; 
  uint32_t net_sizeOfTransfer = htonl(sizeOfTransfer);
  
//  while(targ[sizeOfTransfer]!='\0')
//  {
//    sizeOfTransfer++;
//  }

  send(fd, &net_sizeOfTransfer, sizeof(net_sizeOfTransfer), MSG_MORE);
  send(fd, targ, sizeOfTransfer, 0);
  
  return 0;
}

int my_recv(int fd, char* dest)
{
  uint32_t net_sizeOfTransfer;
  uint32_t sizeOfTransfer;

  recv(fd, &net_sizeOfTransfer, sizeof(net_sizeOfTransfer),0);
  sizeOfTransfer = ntohl(net_sizeOfTransfer);
  recv(fd, dest, sizeOfTransfer, 0);

  return 0;
}


int my_recv_large(int fd, char ** dest)
{
  uint32_t net_sizeOfTransfer;
  uint32_t sizeOfTransfer;

  recv(fd, &net_sizeOfTransfer, sizeof(net_sizeOfTransfer),0);
  sizeOfTransfer = ntohl(net_sizeOfTransfer);

  char * recieved = malloc(sizeOfTransfer);
  recv(fd, recieved, sizeOfTransfer, 0);

  *dest = recieved;
  return 0;
}

