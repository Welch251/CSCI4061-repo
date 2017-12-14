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
  check_fail(ts_fd < 0, 1, "can't open to_server fifo\n");
  char * user_name = args->arg2;					  // Retrieve the username
  mesg_t msg;
  mesg_t *mesg_to_s = &msg;
  memset(mesg_to_s, 0, sizeof(mesg_t));					  // Initialize message struct bytes to default values of 0
  int ret = snprintf(mesg_to_s->name, MAXNAME, "%s", user_name);			  // Write user name to message struct
  check_fail(ret < 0, 1, "mesg_to_s initialization failed\n");
  while(!simpio->end_of_input) {
    simpio_reset(simpio);
    iprintf(simpio, "");                			          // print prompt
    while(!simpio->line_ready && !simpio->end_of_input){		  // Take in user input, one char at a time
      simpio_get_char(simpio);
    }
    if(simpio->line_ready){
      int ret2 = snprintf(mesg_to_s->body, MAXLINE, "%s", simpio->buf); 		  // Once input for a line is complete, print the line to the message struct
      check_fail(ret2 < 0, 1, "mesg_to_s body initialization failed\n");
      mesg_to_s->kind = BL_MESG;					  // Set message kind to appropriate value BL_MESG
      int wr = write(ts_fd, mesg_to_s, sizeof(mesg_t));				  // Send message to to-server FIFO
      check_fail(wr < 0, 1, "write to to_server fifo failed\n");
    }
  }
  // Thread shutdown
  int cancel = pthread_cancel(server_thread); 					  // kill the background thread
  check_fail(cancel != 0, 1, "server thread cancel failed\n");
  mesg_to_s->kind = BL_DEPARTED;					  // Set message kind to appropriate value BL_DEPARTED
  int wr2 = write(ts_fd, mesg_to_s, sizeof(mesg_t));				  // Send DEPART message to to-server FIFO
  check_fail(wr2 < 0, 1, "write to to_server fifo depart failed\n");
  int cl = close(ts_fd);								  // Close the to-server FIFO
  check_fail(cl < 0, 1, "can't close to_server fifo\n");
  return NULL;
}

// Worker thread to listen to the info from the server.
void *server_feed(void *arg){
  char *user_fname_tc = (char *)arg;					// Retrieve the username
  int tc_fd = open(user_fname_tc, O_RDONLY);				// Open the to-client FIFO
  check_fail(tc_fd < 0, 1, "can't open to_client fifo\n");
  while (1){
    mesg_t mesg;
    mesg_t *mesg_to_c = &mesg;
    memset(mesg_to_c, 0, sizeof(mesg_t));				// Initialize message struct bytes to default values of 0
    int rd = read(tc_fd, mesg_to_c, sizeof(mesg_t));				// Read message from to-client fifo into message struct
    check_fail(rd < 0, 1, "read from to_client fifo failed\n");
    char terminal_mesg[MAXLINE];
    if (mesg_to_c->kind == BL_MESG){					// Message kind is MESG
      int ret = snprintf(terminal_mesg, MAXLINE, "[%s] : %s \n", mesg_to_c->name, mesg_to_c->body); // Display the given user's message
      check_fail(ret < 0, 1, "can't initialize MESG message\n");
    }
    else if (mesg_to_c->kind == BL_JOINED){				// Message kind is JOINED
      int ret2 = snprintf(terminal_mesg, MAXLINE, "-- %s JOINED -- \n", mesg_to_c->name);  // Display notification that the given user has joined
      check_fail(ret2 < 0, 1, "can't initialize JOINED message\n");
    }
    else if (mesg_to_c->kind == BL_DEPARTED){				// Message kind is DEPARTED
      int ret3 = snprintf(terminal_mesg, MAXLINE, "-- %s DEPARTED -- \n", mesg_to_c->name); // Display notification that the given user has left
      check_fail(ret3 < 0, 1, "can't initialize DEPARTED message\n");
    }
    else if (mesg_to_c->kind == BL_SHUTDOWN){				// Message kind is SHUTDOWN
      int ret3 = snprintf(terminal_mesg, MAXLINE, "!!! server is shutting down !!!\n");  // Display notification that the server is shutting down
      check_fail(ret3 < 0, 1, "can't initialize SHUTDOWN message\n");
      iprintf(simpio, terminal_mesg);
      break;
    }
    iprintf(simpio, terminal_mesg);					// Ensure that the input prompt is ahead of any message
  }
  int cl = close(tc_fd);								// Close the to-client FIFO
  check_fail(cl < 0, 1, "can't close to_client fifo\n");
  int cancel = pthread_cancel(user_thread);						// kill the background thread
  check_fail(cancel != 0, 1, "user thread cancel failed\n");
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

  int ret = snprintf(join->name, MAXPATH, "%s", user_name); //Send user name info to join struct
  check_fail(ret < 0, 1, "can't initialize join request name\n");
  int ret2 = snprintf(join->to_client_fname, MAXPATH, "%s", user_fname_tc); //Send to-client name info to join struct
  check_fail(ret2 < 0, 1, "can't initialize to_client name\n");
  int ret3 = snprintf(join->to_server_fname, MAXPATH, "%s", user_fname_ts); //Send to-server name info to join struct
  check_fail(ret3 < 0, 1, "can't initialize to_server name\n");

  serv_fd = open(serv_fname, O_RDWR); 		// Open the server's join FIFO, set as READ and WRITE
  check_fail(serv_fd < 0, 1, "can't open join fifo\n");

  struct arg_struct args;			// Initialize struct to contain the two pieces of information needed by the user thread
  args.arg1 = user_fname_ts;
  args.arg2 = user_name;


  int wr = write(serv_fd, join, sizeof(join_t));		//Write join request to server's FIFO
  check_fail(wr < 0, 1, "can't write to join fifo\n");
  char prompt[MAXNAME];
  int prmpt = snprintf(prompt, MAXNAME, "%s>> ",user_name); // create a prompt string
  check_fail(prmpt < 0, 1, "can't initialize prompt\n");
  simpio_set_prompt(simpio, prompt);         // set the prompt
  simpio_reset(simpio);                      // initialize io
  simpio_noncanonical_terminal_mode();       // set the terminal into a compatible mode

  int usr = pthread_create(&user_thread, NULL, user_feed, (void *) &args);		// Create thread for user input
  check_fail(usr != 0, 1, "can't create user thread\n");
  int srv = pthread_create(&server_thread, NULL, server_feed, (void *) user_fname_tc);    // Create thread for server output
  check_fail(srv != 0, 1, "can't create server thread\n");
  int usr_join = pthread_join(user_thread, NULL);						// Regroup user thread following cancellation
  check_fail(usr_join != 0, 1, "can't join user thread\n");
  int srv_join = pthread_join(server_thread, NULL);						// Regroup server thread following cancellation
  check_fail(srv_join != 0, 1, "can't join server thread\n");
  int cl = close(serv_fd);								// Close the server's join FIFO
  check_fail(cl < 0, 1, "can't close join fifo\n");
  simpio_reset_terminal_mode();
  printf("\n");                 // newline just to make returning to the terminal prettier
}
