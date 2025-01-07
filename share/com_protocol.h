//comm_protocol.
#pragma once

#include <string.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>

#define STD_TRANSFER_LEN 10

#define T_MAP  'm'
#define T_PL   'p'
#define T_TIME 't'

typedef struct serialized_game_frame{
  uint32_t map_h;
  uint32_t map_w;
  uint32_t map_char_len;
  char* map;
}game_frame;


int my_send(int fd, char* targ);
int my_recv(int fd, char* dest);

int my_send_large(int fd, char* targ);
int my_recv_large(int fd, char ** dest);



