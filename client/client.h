//client.h
#pragma once

#include <poll.h>
#include <stdatomic.h>

#include "../share/run_param.h"

typedef struct communication_data {
  atomic_int work; 
  struct pollfd fds[2];
  int client_fd;
  //int uniqe_identifier;
} communication_data;

void client_init(communication_data* data);
int connect_to_server (int * client_fd, run_param * rp);
void * communication_task(void * arg);
void com_out_task(struct communication_data* data);
void com_in_task(struct communication_data* data);

void client_destroy(communication_data * data);

//int main (int argc, char* argv[]);

void client_dispache(run_param * rp);

