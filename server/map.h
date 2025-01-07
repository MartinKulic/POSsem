//map.h
#pragma once

#define RESET     "\033[0m"
#define RED_P_C   "\033[1;30;41m"
#define GREEN_P_C "\033[1;30;42m"
#define WHITE_P_C "\033[1;30;47m"
#define BLUE_P_C  "\033[1;30;46m"

#define SNAKE_BODY_CH 'H' //178

#define FRUIT_1_C "\033[1;97;103m"
#define FRUIT_CH 'O'

#define FRUIT_1_CELL (struct map_cell){FRUIT_CH, FRUIT_1_C}


#define BLOCK_C "\033[1;107;107m"
#define BLOCK_CH 'D' //219
#define BLOCK_CELL (struct map_cell){BLOCK_CH, BLOCK_C}

#define MAP_EMPTY ' '
#define EMPTY_CELL (struct map_cell){MAP_EMPTY, RESET}

// min mapa 9x9

typedef struct coord{
  int x;
  int y;
}coord;

typedef struct map_cell{
    char ch;
    char* control;
}map_cell;

typedef struct map{
  map_cell** map;
  map_cell** map_no_players;
  coord dim;
}map;

void map_init(map* this, map_cell default_fill ,coord dim);
void reset_map(map* this);
void clone_map(map_cell** src, map_cell** dest, coord dim);
void print_map(struct map_cell** map, struct coord dim);
_Bool check_colision(map_cell** map, coord p, char player_action);
_Bool try_generate_fruit(map* map);
coord get_coord_for_new_player(map* this);
void map_destroy(map* map);

void serialize_map(map* this, char** target);
