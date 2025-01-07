//map.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "map.h"
#include "../share/com_protocol.h"

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


void reallocate_field(char** field, size_t* alloc_size, size_t increase)
{
  printf("realokujem\n");
  (*alloc_size) = (*alloc_size)+increase;
  (*field) = realloc(*field, *alloc_size);
}
void serialize_map(map* this, char** target)
{
  size_t index = 1;
  size_t alocated_size = ((10+1+3)*this->dim.x*this->dim.y) + 3*this->dim.y + 2*this->dim.x; // (controlChar + char + resetChar) * rozmarMapy + timesNewLine+nocneHovadinky + hornospodneHovadimky
  char* map_flatened = calloc(alocated_size, sizeof(char));
  map_flatened[0] = T_MAP;

  char temp[50] = {'p'};
  size_t len_of_temp;

  len_of_temp = sprintf(&map_flatened[index],"┏");
  index += len_of_temp;
  len_of_temp = sprintf(&temp[0], "━");
  for(size_t i = 0; i < this->dim.x; i++)
  {
    if (index+len_of_temp+1>alocated_size)
    {
      reallocate_field(&map_flatened, &alocated_size, this->dim.x);
    }
    strcpy(&map_flatened[index], &temp[0]);
    index += len_of_temp;
  }
  len_of_temp = sprintf(&temp[0], "┓\n");
  if (index+len_of_temp+1>alocated_size)
  {
    reallocate_field(&map_flatened, &alocated_size, this->dim.x);
  }
  strcpy(&map_flatened[index], &temp[0]);
  index += len_of_temp;

  for(int i = 0; i < this->dim.y; i++)
  {
    len_of_temp = sprintf(&temp[0], "┃");
    if (index+len_of_temp+1>alocated_size)
    {
      reallocate_field(&map_flatened, &alocated_size, this->dim.x);
    }
    strcpy(&map_flatened[index], &temp[0]);
    index += len_of_temp;

    for(int ii = 0; ii < this->dim.x; ii++)
    {
      map_cell mcell = this->map[i][ii];
      if(mcell.ch == MAP_EMPTY)
      {
        if(index+2>alocated_size)
        {
          reallocate_field(&map_flatened, &alocated_size, this->dim.x);
        }
        
        map_flatened[index++] = mcell.ch;
        map_flatened[index] = '\0';
      }
      else{
        //printf("%c %p", temp[0], &temp[0]);
        len_of_temp = (sprintf(&temp[0], "%s%c%s", mcell.control, mcell.ch, RESET));// return the number of bytes printed (excluding the null byte used to end output to strings)
        if (index+len_of_temp+1>alocated_size)
        {
          reallocate_field(&map_flatened, &alocated_size, this->dim.x);
        }
        strcpy(&map_flatened[index], &temp[0]);
        index += len_of_temp;
      }
    }
    len_of_temp = sprintf(&temp[0], "┃\n");
    if (index+len_of_temp+1>alocated_size)
    {
      reallocate_field(&map_flatened, &alocated_size, this->dim.x);
    }
    strcpy(&map_flatened[index], &temp[0]);
    index += len_of_temp;
  }
  len_of_temp = sprintf(&map_flatened[index],"┗");
  index += len_of_temp;
  len_of_temp = sprintf(&temp[0], "━");
  for(size_t i = 0; i < this->dim.x; i++)
  {
    if (index+len_of_temp+1>alocated_size)
    {
      reallocate_field(&map_flatened, &alocated_size, this->dim.x);
    }
    strcpy(&map_flatened[index], &temp[0]);
    index += len_of_temp;
  }
  if (index+len_of_temp+1>alocated_size)
  {
    reallocate_field(&map_flatened, &alocated_size, this->dim.x);
  }
  sprintf(&map_flatened[index], "┛");
  index += len_of_temp;



  *target = map_flatened;
}
