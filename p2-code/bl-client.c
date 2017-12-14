#include "blather.h"

/*
  Authors: Evan Welch & Jakob Urnes
*/


simpio_t simpio_actual;
simpio_t *simpio = &simpio_actual;

client_t client_actual;
client_t *client = &client_actual;

pthread_t user_thread;          // thread managing user input
pthread_t server_thread;	// thread managing server input

struct arg_struct {
    char *arg1;
    char *arg2;
};

typedef enum { false, true } bool;

// Worker thread to manage user input
void *user_feed(void *arg){
  struct arg_struct *args = (struct arg_struct *)arg;
  int ts_fd = open(args->arg1, O_WRONLY);				  // Open the to-server FIFO
  char * user_name = args->arg2;					  // Retrieve the username
  mesg_t msg;
  mesg_t *mesg_to_s = &msg;
  memset(mesg_to_s, 0, sizeof(mesg_t));					  // Initialize message struct bytes to default values of 0
  snprintf(mesg_to_s->name, MAXNAME, "%s", user_name);			  // Write user name to message struct
  while(!simpio->end_of_input) {
    simpio_reset(simpio);
    iprintf(simpio, "");                			          // print prompt
    while(!simpio->line_ready && !simpio->end_of_input){		  // Take in user input, one char at a time
      simpio_get_char(simpio);
    }
    if(simpio->line_ready){
      snprintf(mesg_to_s->body, MAXLINE, "%s", simpio->buf); 		  // Once input for a line is complete, print the line to the message struct
      mesg_to_s->kind = BL_MESG;					  // Set message kind to appropriate value BL_MESG
      write(ts_fd, mesg_to_s, sizeof(mesg_t));				  // Send message to to-server FIFO
    }
  }
  // Thread shutdown
  pthread_cancel(server_thread); 					  // kill the background thread
  mesg_to_s->kind = BL_DEPARTED;					  // Set message kind to appropriate value BL_DEPARTED
  write(ts_fd, mesg_to_s, sizeof(mesg_t));				  // Send DEPART message to to-server FIFO
  close(ts_fd);								  // Close the to-server FIFO
  return NULL;
}

// Worker thread to listen to the info from the server.
void *server_feed(void *arg){
  char *user_fname_tc = (char *)arg;					// Retrieve the username
  int tc_fd = open(user_fname_tc, O_RDONLY);				// Open the to-client FIFO
  while (1){
    mesg_t mesg;
    mesg_t *mesg_to_c = &mesg;
    memset(mesg_to_c, 0, sizeof(mesg_t));				// Initialize message struct bytes to default values of 0
    read(tc_fd, mesg_to_c, sizeof(mesg_t));				// Read message from to-server client into message struct
    char terminal_mesg[MAXLINE];
    if (mesg_to_c->kind == BL_MESG){					// Message kind is MESG
      snprintf(terminal_mesg, MAXLINE, "[%s] : %s \n", mesg_to_c->name, mesg_to_c->body); // Display the given user's message
    }
    else if (mesg_to_c->kind == BL_JOINED){				// Message kind is JOINED
      snprintf(terminal_mesg, MAXLINE, "-- %s JOINED -- \n", mesg_to_c->name);  // Display notification that the given user has joined
    }
    else if (mesg_to_c->kind == BL_DEPARTED){				// Message kind is DEPARTED
      snprintf(terminal_mesg, MAXLINE, "-- %s DEPARTED -- \n", mesg_to_c->name); // Display notification that the given user has left
    }
    else if (mesg_to_c->kind == BL_SHUTDOWN){				// Message kind is SHUTDOWN
      snprintf(terminal_mesg, MAXLINE, "!!! server is shutting down !!!\n");  // Display notification that the server is shutting down
      iprintf(simpio, terminal_mesg);
      break;
    }
    iprintf(simpio, terminal_mesg);					// Ensure that the input prompt is ahead of any message 
  }
  close(tc_fd);								// Close the to-client FIFO
  pthread_cancel(user_thread);						// kill the background thread
  return NULL;
}


int main(int argc, char *argv[]){

  if(argc != 3){				//Check if correct number of arguments has been entered,
    printf("usage: %s <program> <int>\n",argv[0]); // if not, notify user and exit
    exit(0);
  }



  char *serv_fname = argv[1];			//Retrieve name info from input
  char *user_name = argv[2];

  char user_fname_tc[MAXPATH];			//Char arrays that will contain the full fifo names
  char user_fname_ts[MAXPATH];

  //bool shutdown = false;			//Status on whether or not server has shut down

  //bool user;

  int serv_fd;					//File descriptor for server FIFO

  strcpy(user_fname_tc,"");			//Properly set up names of FIFOs in relation to entered username

  strcpy(user_fname_ts,"");
  strcat(user_fname_tc,user_name);
  strcat(user_fname_tc,"_to_cl \0");
  strcat(user_fname_ts,user_name);
  strcat(user_fname_ts,"_to_srv \0");
  strcat(user_name,"\0");

  mkfifo(user_fname_tc, S_IRUSR | S_IWUSR);	//Make to-client FIFO
  mkfifo(user_fname_ts, S_IRUSR | S_IWUSR);     //Make to-server FIFO


  join_t join_actual;
  join_t *join = &join_actual;					//Create join request struct

  memset(join, 0, sizeof(join_t));

  snprintf(join->name, MAXPATH, "%s", user_name); //Send user name info to join struct
  snprintf(join->to_client_fname, MAXPATH, "%s", user_fname_tc); //Send to-client name info to join struct
  snprintf(join->to_server_fname, MAXPATH, "%s", user_fname_ts); //Send to-server name info to join struct

  serv_fd = open(serv_fname, O_RDWR); 		// Open the server's join FIFO, set as READ and WRITE

  struct arg_struct args;			// Initialize struct to contain the two pieces of information needed by the user thread
  args.arg1 = user_fname_ts;
  args.arg2 = user_name;


  write(serv_fd, join, sizeof(join_t));		//Write join request to server's FIFO
  char prompt[MAXNAME];
  snprintf(prompt, MAXNAME, "%s>> ",user_name); // create a prompt string
  simpio_set_prompt(simpio, prompt);         // set the prompt
  simpio_reset(simpio);                      // initialize io
  simpio_noncanonical_terminal_mode();       // set the terminal into a compatible mode

  pthread_create(&user_thread, NULL, user_feed, (void *) &args);		// Create thread for user input
  pthread_create(&server_thread, NULL, server_feed, (void *) user_fname_tc);    // Create thread for server output
  pthread_join(user_thread, NULL);						// Regroup user thread following cancellation
  pthread_join(server_thread, NULL);						// Regroup server thread following cancellation
  close(serv_fd);								// Close the server's join FIFO
  simpio_reset_terminal_mode();
  printf("\n");                 // newline just to make returning to the terminal prettier
}

