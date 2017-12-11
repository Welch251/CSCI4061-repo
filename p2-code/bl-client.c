#include "blather.h"

simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;

client_t client_actual;
client_t *client = &client_actual;

pthread_t user_thread;          // thread managing user input
pthread_t server_thread;	// thread managing server input

struct arg_struct {
    int arg1;
    char *arg2;
};

typedef enum { false, true } bool;

// Worker thread to manage user input
void *user_feed(void *arg){
  struct arg_struct *args = (struct arg_struct *)arg;
  int ts_fd = args->arg1;
  char * user_name = args->arg2;
  mesg_t msg;
  mesg_t *mesg_to_s = &msg;
  snprintf(mesg_to_s->name, MAXNAME, "%s", user_name);
  while(!simpio->end_of_input) {
    simpio_reset(simpio);
    iprintf(simpio, "");                                          // print prompt
    while(!simpio->line_ready && !simpio->end_of_input){          // read until line is complete
      simpio_get_char(simpio);
    }
    if(simpio->line_ready){
      mesg_t mesg;
      mesg_t *mesg_to_server = &mesg;
      snprintf(mesg_to_server->body, MAXLINE, "%s", simpio->buf);
      mesg_to_server->kind = BL_MESG;
      write(ts_fd, mesg_to_server, sizeof(mesg_t));
    }
  }

  pthread_cancel(server_thread); // kill the background thread
  return NULL;
}

// Worker thread to listen to the info from the server.
void *server_feed(void *arg){
  struct arg_struct *args = (struct arg_struct *)arg;
  int tc_fd = args->arg1;
  while (1){
    mesg_t mesg;
    mesg_t *mesg_to_c = &mesg;
    read(tc_fd, mesg_to_c, sizeof(mesg_t));
    char terminal_mesg[MAXLINE];
    if (mesg_to_c->kind == BL_MESG){
      snprintf(terminal_mesg, MAXLINE, "[%s]: %s", mesg_to_c->name, mesg_to_c->body);
    }
    else if (mesg_to_c->kind == BL_JOINED){
      snprintf(terminal_mesg, MAXLINE, "-- %s JOINED --", mesg_to_c->name);
    }
    else if (mesg_to_c->kind == BL_DEPARTED){
      snprintf(terminal_mesg, MAXLINE, "-- %s DEPARTED --", mesg_to_c->name);
    }
    else if (mesg_to_c->kind == BL_SHUTDOWN){
      snprintf(terminal_mesg, MAXLINE, "!!! sunnyvale is shutting down !!!");
    }
    iprintf(simpio, terminal_mesg);
  }
  return NULL;
}


int main(int argc, char *argv[]){
  char *serv_fname = argv[1];			//Retrieve name info from input
  char *user_name = argv[2];
  char user_fname_tc[MAXPATH];			//Char arrays that will contain the full fifo names
  char user_fname_ts[MAXPATH];

  //bool shutdown = false;			//Status on whether or not server has shut down

  //bool user;

  int serv_fd;					//File descriptor for server FIFO
  int tc_fd;					//File descriptor for to-client FIFO
  int ts_fd;					//File descriptor for to-server FIFO

  strcpy(user_fname_tc,"");			//Add name info to char arrays
  strcpy(user_fname_ts,"");
  strcat(user_fname_tc,user_name);
  strcat(user_fname_tc,"_to_cl");
  strcat(user_fname_ts,user_name);
  strcat(user_fname_ts,"_to_srv");

  mkfifo(user_fname_tc, S_IRUSR | S_IWUSR);	//Make to-client FIFO
  mkfifo(user_fname_ts, S_IRUSR | S_IWUSR);     //Make to-server FIFO

  join_t join_actual;
  join_t *join = &join_actual;					//Create join request struct

  snprintf(join->name, MAXPATH, "%s", user_name); //Send user name info to join struct
  snprintf(join->to_client_fname, MAXPATH, "%s", user_fname_tc); //Send to-client name info to join struct
  snprintf(join->to_server_fname, MAXPATH, "%s", user_fname_ts); //Send to-server name info to join struct

  serv_fd = open(serv_fname, O_RDWR); 		//Open the server's FIFO
  tc_fd = open(user_fname_tc, O_RDONLY);
  ts_fd = open(user_fname_ts, O_WRONLY);


  struct arg_struct args;
  args.arg1 = ts_fd;
  args.arg2 = user_name;

  struct arg_struct args2;
  args2.arg1 = tc_fd;
  args2.arg2 = serv_fname;

  write(serv_fd, join, sizeof(join_t));		//Write join request to server's FIFO

  char prompt[MAXNAME];
  snprintf(prompt, MAXNAME, "%s>> ",user_name); // create a prompt string
  simpio_set_prompt(simpio, prompt);         // set the prompt
  simpio_reset(simpio);                      // initialize io
  simpio_noncanonical_terminal_mode();       // set the terminal into a compatible mode


  pthread_create(&user_thread, NULL, user_feed, (void *) &args);
  pthread_create(&server_thread, NULL, server_feed, (void *) &args2);
  pthread_join(user_thread, NULL);
  pthread_join(server_thread, NULL);

  simpio_reset_terminal_mode();
  printf("\n");                 // newline just to make returning to the terminal prettier

}
