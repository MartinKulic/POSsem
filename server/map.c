//map.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "map.h"

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
  for(size_t i = 0; i < dim.y; i++)
  {
    for(size_t ii = 0; ii < dim.x; ii++)
    {
      printf("%s%c%s", map[i][ii].control, map[i][ii].ch, RESET);
    }
    printf("\n");
  }
  printf("-------------------------\n\n");
}

_Bool check_colision(map_cell** map, coord head_pos, char player_action)
{
  //coord head_pos = *(player->body->head_->data_);
  return map[head_pos.y][head_pos.x].ch != player_action;
}

_Bool try_generate_fruit(map_cell** map_p, map_cell ** map_n_p, coord dim)
{
  for(int i = 0; i < 1000000; i++)
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

void map_init(map_cell** this, map_cell default_fill ,coord dim)
{
  this = malloc(dim.y * sizeof(map_cell*));//riadky
  for(size_t i = 0; i < dim.y; i++)
  {
    this[i] = malloc(dim.x * sizeof(map_cell));//znaky v riadkoch
    memset(this[i], 'p', dim.x);
    for(size_t  ii = 0; i < dim.x; i++)
    {
        this[i][ii] = default_fill;
    }
  }
}

void map_destroy(map_cell** this, coord dim)
{
  for(size_t i = 0; i < dim.y; i++)
  {
    free(this[i]);
  }
  free(this);
}
