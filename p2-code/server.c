#include "blather.h"
#include <errno.h>

/*
  Authors: Evan Welch & Jakob Urnes
*/

client_t *server_get_client(server_t *server, int idx){
  return &server->client[idx];						// Return client information at specified index
}
// Initialize the server and its members
void server_start(server_t *server, char *server_name, int perms){
  int ret = snprintf(server->server_name, MAXPATH, "%s", server_name);  // Set the server's name to the argument specified by the user
  check_fail(ret < 0, 1, "server_name initialization failed\n");        // Check if snprintf() system call returned properly
  int rm = remove(server->server_name);					// Remove any prior creations of files of name server_name
  if(rm < 0){								// Check if the remove() system call returned properly
    fprintf(stderr, "can't remove server name file\n");			// If not, notify user
  }
  int ret2 = mkfifo(server->server_name, perms);			// Make the server's join FIFO
  check_fail(ret2 < 0, 1, "fifo can't be made\n");			// Check if the mkfifo() system call returned properly
  server->join_fd = open(server->server_name, O_RDWR);			// Open the server's join FIFO
  check_fail(server->join_fd < 0, 1, "join fifo can't be opened\n");	// Check if the open() system call returned properly
  server->join_ready = 0;						// Initialize # of clients and join_ready status to 0
  server->n_clients = 0;
}
// Stop the server and close its members
void server_shutdown(server_t *server){
  int cl = close(server->join_fd);					// Close the server's join FIFO
  check_fail(cl < 0, 1, "join fifo can't be closed\n");			// Check if close() system call returned properly
  int rm = remove(server->server_name);					// Remove the server's join FIFO
  check_fail(rm < 0, 1, "server name file can't be removed\n");		// Check if the remove() system call returned properly
  mesg_t msg;								// Initialize the message struct
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));					// Set message struct's bytes to default values of 0
  mesg->kind = BL_SHUTDOWN;						// Set message kind to SHUTDOWN
  while(server->n_clients > 0){						// Loop through all clients
    client_t *client = server_get_client(server, server->n_clients-1);	
    int ret = write(client->to_client_fd, mesg, sizeof(mesg_t));        // Write SHUTDOWN message to each client
    check_fail(ret < 0, 1, "server shutdown write to client failed\n");	// Check if the write() system call returned properly
    client = NULL;							// Set client's address to NULL
    server->n_clients--;						// Decrement # of registered clients
  }
}
// Add a new client to the server
int server_add_client(server_t *server, join_t *join){
  client_t client_actual;						// Initialize new client struct
  client_t *client = &client_actual;
  memset(client, 0, sizeof(client_t));					// Initialize the client struct's bytes to default values of 0
  int ret1 = snprintf(client->name, MAXPATH, "%s", join->name);		// Set client's name to the name specified in the join struct
  check_fail(ret1 < 0, 1, "client name initialization failed\n");       // Check if snprintf() system call returned properly
  int ret2 = snprintf(client->to_client_fname, MAXPATH, "%s", join->to_client_fname); // Set the to-client FIFO's name to that in the join struct
  check_fail(ret2 < 0, 1, "to_client_fname initialization failed\n");	// Check if snprintf() system call returned properly
  int ret3 = snprintf(client->to_server_fname, MAXPATH, "%s", join->to_server_fname); //Set the to-server FIFO's name to that in the join struct
  check_fail(ret3 < 0, 1, "to_server_fname initialization failed\n");	// Check if snprintf() system call returned properly
  client->to_client_fd = open(client->to_client_fname, O_WRONLY);	// Open the user's to-client FIFO
  check_fail(client->to_client_fd < 0, 1, "to client fifo can't be opened\n"); // Check if open() system call returned properly
  client->to_server_fd = open(client->to_server_fname, O_RDONLY);	// Open the usere's to-server FIFO
  check_fail(client->to_server_fd < 0, 1, "to server fifo can't be opened\n"); // Check if open() system call returned properly
  client->data_ready = 0;						// Initialize the data_ready status to 0
  if(server->n_clients == MAXCLIENTS){					// Check if the number of clients has reached the max limit
    return 1;								// If so, return 1 without adding the client to the list
  }
  server->client[server->n_clients] = *client;				// If not, add client to the list and increment the client count
  server->n_clients++;
  return 0;
}
// Remove the idx-th client from the server
int server_remove_client(server_t *server, int idx){
  client_t *client = server_get_client(server, idx);			// Retrieve client info
  int cl1 = close(client->to_client_fd);				// Close the client's to-client FIFO
  check_fail(cl1 < 0, 1, "to client fifo can't be closed\n");		// Check if close() system call returned properly
  int cl2 = close(client->to_server_fd);				// Close the client's to-server FIFO
  check_fail(cl2 < 0, 1, "to server fifo can't be closed\n");		// Check if close() system call returned properly
  int rm1 = remove(client->to_client_fname);				// Remove the client's to-client FIFO
  check_fail(rm1 < 0, 1, "to client name file can't be removed\n");	// Check if remove() system call returned properly
  int rm2 = remove(client->to_server_fname);				// Remove the client's to-server FIFO
  check_fail(rm2 < 0, 1, "to server name file can't be removed\n");	// Check if remove() system call returned properly
  for(int i = idx+1; i < server->n_clients; i++){
    server->client[i-1] = *server_get_client(server, i);		// If client is in center of list, move the client information to keep occupied spaces consecutive
  }
  server->n_clients--;							// Decrement the # of clients
  return 0;
}
// Broadcast a message to all clients
int server_broadcast(server_t *server, mesg_t *mesg){
  for(int i = 0; i < server->n_clients; i++){				// Loop through each client in the serer
    client_t *client = server_get_client(server, i);			// Get the client's information
    int ret = write(client->to_client_fd, mesg, sizeof(mesg_t));	// Write the message to the client's to-client FIFO
    check_fail(ret < 0, 1, "write to client %d failed\n", i);		// Check if write() system call returned properly
  }
  return 0;
}
// Check for any new join requests or client data that is ready to be handled
void server_check_sources(server_t *server){
  fd_set fds;								// Create an fd set containing the join struct
  FD_ZERO(&fds);
  int max_fd = server->join_fd;						// Retrieve the max fd for the select call
  FD_SET(server->join_fd, &fds);					// Add join struct to fd set
  for(int i = 0; i < server->n_clients; i++){
    client_t *client = server_get_client(server, i);			// Get client information
    FD_SET(client->to_server_fd, &fds);					// Add client's to-server FIFO to fd set
    if(client->to_server_fd > max_fd){					// If the most recent FIFO has the highest fd #, set it as the max fd
      max_fd = client->to_server_fd;
    }
  }
  max_fd++;								// increment max_fd by one so it ahead of highest fd
  int ret = select(max_fd, &fds, NULL, NULL, NULL);			// Checks if any data is ready on the entered FIFOs
  if(errno == EINTR){
    fprintf(stderr, "the select for server_check_sources received a signal and exited\n");
    return;
  } else{
      check_fail(ret < 0, 1, "select call failed\n");			// Check if select() system call returned properly
      // At least one file descriptor has data ready
      if(FD_ISSET(server->join_fd, &fds)){				// If the join_fd has ready information, set join_ready flag to 1
        server->join_ready = 1;
      }
      for(int i = 0; i < server->n_clients; i++){			// If any client has ready information, set its data_ready flag to 1
        client_t *client = server_get_client(server, i);
        if(FD_ISSET(client->to_server_fd, &fds)){
          client->data_ready = 1;
        }
      }
  }
}
// Check if the server's join struct has data that is ready
int server_join_ready(server_t *server){
  return server->join_ready;
}
// Handle a client's join request
int server_handle_join(server_t *server){
  join_t join;								// Initialize a new join struct
  int ret = read(server->join_fd, &join, sizeof(join_t));		// Read info from server's join FIFO into the join struct
  check_fail(ret < 0, 1, "read for join fifo failed\n");		// Check if read() system call returned properly
  server_add_client(server, &join);					// Add client info to the server
  client_t *client = server_get_client(server, server->n_clients-1);	// Retrieve new client info
  mesg_t msg;								// Initialize a message struct
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));					// Initialize message struct's bytes to default values of 0
  int ret2 = snprintf(mesg->name, MAXNAME, "%s", client->name);		// Set message struct's user name to that of given client
  check_fail(ret2 < 0, 1, "message name initialization failed\n");	// Check if snprintf() system call returned properly
  mesg->kind = BL_JOINED;						// Set the message's kind to JOINED					
  server_broadcast(server, mesg);					// Broadcast the JOINED message to all users
  server->join_ready = 0;						// Reset the server's join_ready flag to 0
  return 0;
}
// Check if the idx-th client has data that is ready
int server_client_ready(server_t *server, int idx){
  client_t *client = server_get_client(server, idx);			// Retrieve client information at specified index
  return client->data_ready;						// Return the client's data_ready flag status
}
// Handle the idx-th client's message request
int server_handle_client(server_t *server, int idx){
  mesg_t msg;								// Initialize a new message struct
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));					// Initialize message struct's bytes to default values of 0
  client_t *client = server_get_client(server, idx);			// Retrieve client's information
  int ret = read(client->to_server_fd, mesg, sizeof(mesg_t));		// Read the client's message information into the message struct
  check_fail(ret < 0, 1, "read for to server fifo failed\n");		// Check if read() system call returned properly
  if(mesg->kind == BL_MESG){						// Message kind is MESG
    server_broadcast(server, mesg);					// Broadcast message to all clients
    client->data_ready = 0;						// Reset client's data_ready flag to 0
  } else if(mesg->kind == BL_DEPARTED) {				// Message kind is DEPARTED 
      server_remove_client(server, idx);				// Remove the client from the server
      server_broadcast(server, mesg);					// Broadcast client's DEPART message to all clients
  }
  return 0;
}
