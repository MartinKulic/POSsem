//player.c

#include "player.h"

void serialize_players(sll * players, int serv_time, char ** destination)
{
  size_t index = 1;
  size_t pomLen = 0;
  char* t1 = "┢━━━━━━━━━━━━━━╍┅┉┈\n";
  size_t t1_size = strlen(t1);
  char* t2 = "┣━━━━━━━━━━━━━━╍┅┉┈\n";
  char* t3 = "┗━━━━━━━━━━━━━━╍┅┉┈\n";
  char* dest = malloc((50*sll_get_size(players)+(t1_size*3))+100);
  dest[0] = 'f';
  
  pomLen = sprintf(&dest[index], "┊  %d\n%s", serv_time, t1);
  index += pomLen;

  sll_node * node = players->head_;

  if( node != NULL )
  {
    
    while(node != NULL)
    {
      player * p = *(player**)node->data_;
      pomLen = sprintf(&dest[index], "┃%sH:>\033[0m  %d\n%s", p->colour, p->scor,t2);
      index += pomLen;
      node = node->next_;
    }
  }

  sprintf(&dest[index], "%s\n", t3);
  *destination = dest;
}

