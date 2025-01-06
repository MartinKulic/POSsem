//map.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "map.h"

void reset_map(map* this)
{
  clone_map(this->map_no_players, this->map, this->dim);
}

void clone_map(map_cell** src, map_cell** dest, coord dim)
{
  for(size_t i = 0; i < dim.y; i++)
  {
    memcpy(dest[i], src[i], dim.x*sizeof(struct map_cell));//nemali by sa nikdy prekrivat
  }
}
void print_map(map_cell** map, struct coord dim)
{
  printf("\n");
  printf("┏");
  for(size_t i = 0; i < dim.x; i++)
  {
    printf("━");
  }
  printf("┓\n");
  for(size_t i = 0; i < dim.y; i++)
  {
    printf("┃");
    for(size_t ii = 0; ii < dim.x; ii++)
    {
      printf("%s%c%s", map[i][ii].control, map[i][ii].ch, RESET);
    }
    printf("┃\n");
  }
  printf("┗");
  for(size_t i = 0; i < dim.x; i++)
  {
    printf("━");
  }
  printf("┛\n");
}

_Bool check_colision(map_cell** map, coord head_pos, char player_action)
{
  //coord head_pos = *(player->body->head_->data_);
  return map[head_pos.y][head_pos.x].ch != player_action;
}

_Bool try_generate_fruit(map * this)
{
  map_cell** map_p = this->map;
  map_cell** map_n_p = this->map_no_players;
  coord dim = this->dim;

  for(int i = 0; i < 100000; i++)
  {
    size_t x = rand()%dim.x;
    size_t y = rand()%dim.y;

    if(map_p[y][x].ch == MAP_EMPTY)
    {
      map_n_p[y][x] = FRUIT_1_CELL;
      return 1;
      break;
    }
  }
  return 0;
}
coord get_coord_for_new_player(struct map* this)
{
  int x = 5;
  int y = 5;

  while(this->map[y][x].ch != MAP_EMPTY)
  { 
    x = rand()%(this->dim.x-9)+5;
    y = rand()%(this->dim.y-9)+5;
  }

  return (coord){x,y};
}
void map_init(map* this, map_cell default_fill ,coord dim)
{
  this->dim = dim;
  this->map = malloc(dim.y * sizeof(map_cell*));//riadky
  this->map_no_players = malloc(dim.y * sizeof(map_cell*));
  for(size_t i = 0; i < dim.y; i++)
  {
    this->map[i] = malloc(dim.x * sizeof(map_cell));//znaky v riadkoch
    this->map_no_players[i] = malloc(dim.x * sizeof(map_cell));

    for(size_t  ii = 0; ii < dim.x; ii++)
    {
        this->map[i][ii] = default_fill;
        this->map_no_players[i][ii] = default_fill;
    }
  }
}

void map_destroy(map* this)
{
  for(size_t i = 0; i < this->dim.y; i++)
  {
    free(this->map[i]);
    free(this->map_no_players[i]);
  }
  free(this->map);
  free(this->map_no_players);
}
