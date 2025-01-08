//menu.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include "run.h"
#include "../client/client.h"
#include "../server/server.h"


int loop()
{
  _Bool work = 1;
  run_param_all_char run_p_all_char = {"127.0.0.1", "8080", "", ""};
  run_param run_param = {&run_p_all_char.ip[0], 8080, -1, 5, &run_p_all_char.path_to_map[0], 50, 25};
  printf("VYTAJ\n");
  while (work)
  {
    print_menu();

    char moznost = get_single_char();

    switch (moznost) {
      case '1':
        make_new_game(&run_param);
      break;
      case '2':
        connect_to_game(&run_param);
      break;
      case '3':
        work = 0;
      break;
      default:
        printf("\n\033[1;37;31mNerozpoznana volba\033[0m\n");
      break;
    }
  }
}

void make_new_game(run_param * rp)
{
  int next = 1;
  rp->ip = "127.0.0.1";
  char temp[INT32_BUFF_SIZE];
  int tmpint;

  while(next > 0)
  {
    switch (next)
    {
      case 1:
        sprintf(&temp[0], "%d", rp->port);
        next = do_editable_thing("Port: ", &temp[0], PORT_BUFF_SIZE) ? -1 : next+1;
        rp->port = atoi(temp);
      break;
      case 2:
        sprintf(&temp[0], "%d", rp->game_time);
        char message[100];
        sprintf(&message[0], "Cas trvania hry (<= %d = neobmedzeny): ", GAME_NO_TIME_LIMIT);
        next = do_editable_thing(&message[0], &temp[0], INT32_BUFF_SIZE) ? next-1 : next+1;
        rp->game_time = atoi(temp);
      break;
      case 3:
        sprintf(&temp[0], "%d", rp->max_players);
        next = do_editable_thing("Maximalny pocet pripojenych hracov: ", &temp[0], INT32_BUFF_SIZE) ? next-1 : next+1;
        rp->max_players = atoi(&temp[0]);
      break;
      case 4:
        next = do_editable_thing("Cesta k mape (prazdne = mapa bez prekazok): ", rp->path_to_map, PATH_T_MAP_BUFF_SIZE) ? next-1 : next+1;
        if (rp->path_to_map[0]!='\0') {next = 0;}
      break;
      case 5:
        sprintf(&temp[0], "%d", rp->map_w);
        next = do_editable_thing("Sirka mapy: ", &temp[0], INT32_BUFF_SIZE) ? next-1 : next+1;
        tmpint = atoi(&temp[0]);
        if(tmpint < 9){
          next = 5;
          printf("\033[0,31mMinimalny rozmer mapy je 9\033[0m\n");
        }
        else{
          rp->map_w = tmpint;
        }
      break;
      case 6:
        sprintf(&temp[0], "%d", rp->map_h);
        next = do_editable_thing("Vyska mapy: ", &temp[0], INT32_BUFF_SIZE) ? next-1 : next+1;
        tmpint = atoi(&temp[0]);
        if(tmpint < 9){
          next = 5;
          printf("\033[0,31mMinimalny rozmer mapy je 9\033[0m\n");
        }
        else{
          rp->map_h = tmpint;
        }
      break;
      default:
        next = 0;
      break;
    }
  }
  if(next == -1) // akcia zrusena
  { 
    return;
  }
  
  pid_t pid = fork();

  if(pid == 0) //children
  {
    do_server_stuff(rp);
  }
  else
  {
    printf("waiting for server\n");
    sleep(1);
    do_client_stuff(rp);
  }

}

void do_server_stuff(run_param * rp)
{
  server_dispache(rp);
  exit(0);
}

void connect_to_game(run_param * rp)
{
  int next = 1;
  char temp[INT32_BUFF_SIZE];
  int tmpint;

  while(next > 0)
  {
    switch (next)
    {
      case 1:
        next = do_editable_thing("Server ip address: ", rp->ip, IP_BUFF_SIZE) ? -1 : next+1;
      break;
      case 2:
        sprintf(&temp[0], "%d", rp->port);
        next = do_editable_thing("Port: ", &temp[0], PORT_BUFF_SIZE) ? next-1 : next+1;
        rp->port = atoi(temp);
      break;
      default:
        next = 0;
      break;
    }
  }

  if(next == -1){
    return;
  }

  do_client_stuff(rp);

}
void do_client_stuff(run_param * rp)
{
  //client_dispache(rp);
  _Bool not_ended = 1;
  communication_data * this = calloc(1, sizeof(communication_data));
  client_init(this);
  if (connect_to_server(&this->client_fd, rp) != 1)
  {
    printf("\033[1;5;30;41mNepodarilo sa pripojit na server!\033[0m\n\033[3;37mUisti sa, ze adresa a port su spravne.\033[0m\n");
    this->client_fd = -1;
    client_destroy(this);
    return ;
  }
  while(not_ended)
  {
    this->work = 1;
    communication_task(this);
    if(this->work != 2) //server did not ended
    {
      not_ended = pause_menu();
    }
    else
    {
      printf("Game ended\n");
      not_ended = 0;
    }
  }
  client_destroy(this);
}

_Bool pause_menu()
{
  printf("\033[3;37mPaused\033[0m\n");
  printf("\033[1m1\033[0m - \033[4mPokracovat\033[0m\n\033[1m2\033[0m - \033[4mUkonncit\033[0m\n");
  
  char ch;
  while(1)
  {
    ch = get_single_char();
    switch(ch)
    {
      case '1':
        return 1;
      break;
      case '2':
        return 0;
      break;
      default:
        printf("\n\033[1;37;31mNerozpoznana volba\033[0m\n");
      break;
    }
  }
}

char get_single_char()
{
  char ch;
  while (1)
  {
    read(STDIN_FILENO, &ch, 1);
    if (ch >= 32 && ch <= 126)
    {
      break;
    }
  }
  return ch;
}

_Bool do_editable_thing(char * popis, char * d_value, size_t val_buff_size)
{
  struct pollfd fds[1];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  char value[val_buff_size];
  strcpy(&value[0], d_value);

  printf("%s %s", popis, value);
  fflush(stdout);
 
  int pos = strlen(d_value);
  char ch;
  while (read(STDIN_FILENO, &ch, 1) > 0)
  {
    if (ch == 27)
    {
      if (poll(fds, 1, 5) > 0 && fds[0].revents == POLLIN)
      {
        read(STDIN_FILENO, &ch, 1);
        if (poll(fds, 1, 5) > 0)
        {
          read(STDIN_FILENO, &ch, 1);
          if (ch == 'D') // zmaz cely riadok
          {
            for(;pos > 0; pos--)
            {
              printf("\b \b");
            }
            fflush(stdout);
            value[pos] = '\0';
          }
        }
      }
      else{ // esc
        printf("\n");
        return 1;
      }
    }

    else if (ch == '\n') // enter
    {
      value[pos] = '\0';
      break;
    }
    else if ((ch == 127 || ch == '\b') && pos > 0) // backspace
    {
      pos--;
      value[pos] = '\0';
      printf("\b \b");
      fflush(stdout);
    }
    else if (ch >= 32 && ch <= 126)
    {
      if(pos < val_buff_size - 1)
      {
        value[pos++] = ch;
        value[pos] = '\0';
        printf("%c", ch);
        fflush(stdout);
      }
    }
  }
  strcpy(d_value, &value[0]);
  printf("\n");
  return 0;
}

void print_menu()
{
  printf("\033[1m1\033[0m - \033[4mNova Hra\033[1;24m\n2\033[0m - \033[4mPripojit k hre\033[1;24m\n3\033[0m - \033[4mKoniec\033[0m\n");
}

int main(int argc, char *argv[])
{
  //treminal to raw
  struct termios oldt, newt;
  tcgetattr(0, &oldt);
  memcpy(&newt, &oldt, sizeof(struct termios));
  newt.c_lflag &= ~(ICANON | ECHO);
  
  newt.c_cc[VTIME] = 100; // timeot in deciseconds for noncanonical read
  
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); 

  loop();

  //reset terminal
  tcsetattr(0, TCSANOW, &oldt);

  return 0;
}
