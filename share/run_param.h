//run_param.h
#pragma once

#define IP_BUFF_SIZE 16
#define PORT_BUFF_SIZE 6
#define INT32_BUFF_SIZE 11
#define PATH_T_MAP_BUFF_SIZE 500

#define GAME_NO_TIME_LIMIT -1

typedef struct run_param{
  char * ip;
  int port;
  int game_time; //duration of game
  int max_players;
  char * path_to_map;
  int map_w;
  int map_h;
} run_param;

typedef struct run_param_all_char {
  char ip[IP_BUFF_SIZE];
  char port[PORT_BUFF_SIZE];
  char game_time[INT32_BUFF_SIZE];
  char path_to_map[PATH_T_MAP_BUFF_SIZE];
  }run_param_all_char ;
