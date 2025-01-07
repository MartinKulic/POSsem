//menu.h

#include "../share/run_param.h"

int loop();
char get_single_char();
_Bool do_editable_thing(char * popis, char * value, size_t val_buff_size);
void print_menu();
void make_new_game(run_param * rp);
void do_server_stuff(run_param * rp);
void connect_to_game(run_param * rp);
void do_client_stuff(run_param * rp);

int main(int argc, char *argv[]);
