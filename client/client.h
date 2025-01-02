//client.h
#pragma once


#include <poll.h>
#include <stdatomic.h>

typedef struct communication_data {
  atomic_bool work; 
  struct pollfd fds[2];
  int client_fd;
  int uniqe_identifier;
} communication_data;


int connect_to_server (int * client_fd, int * uniqe_identifier);
void * communication_task(void * arg);
void com_out_task(struct communication_data* data);
void com_in_task(struct communication_data* data);

int main (int argc, char* argv[]);

