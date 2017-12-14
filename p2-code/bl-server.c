#include "blather.h"

/*
  Authors: Evan Welch & Jakob Urnes
*/

int signalled = 0;

void sig_handler(int signo) {
  signalled = 1;						// Signifies that loop should stop
}

int main(int argc, char *argv[]){
  if(argc != 2){
    printf("usage: %s <server name> \n",argv[0]);		// Check if the correct number of arguments was entered,
    exit(0);							// if not, notify user and exit 
  }

  char * serv_fname = argv[1];					// Sets the server's name equal to the first argument
  server_t server;
  server_t *serv = &server;					// Initialize the server struct
  memset(serv, 0, sizeof(server_t));
  server_start(serv, serv_fname, DEFAULT_PERMS);
  signal(SIGTERM, sig_handler);					// Replace default SIGTERM disposition with the sig_handler call
  signal(SIGINT, sig_handler);					// Replace default SIGINT disposition with the sig_handler call
  while (!signalled){						// Continue through loop as long as not signalled, allows for graceful shutdown
    server_check_sources(serv);					// Check if there are any new join requests
    if(server_join_ready(serv)){				// If so, handle the join request and add the client to the server
      server_handle_join(serv);
    }
    for (int i=0; i < serv->n_clients; i++){			// Go through the list of registered clients, check if they have 
      if(server_client_ready(serv, i)){                         // new data that is ready to be processed
        server_handle_client(serv,i);
      }
    }
  }
  server_shutdown(serv);                                       // Call shutdown sequence for server
  return 0;
}

