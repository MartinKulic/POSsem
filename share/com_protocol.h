//comm_protocol.
#pragma once

#include <string.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdio.h>

#define STD_TRANSFER_LEN 10

int my_send(int fd, char* targ);
int my_recv(int fd, char* dest);


