<<<<<<< HEAD
#include "blather.h"

client_t *server_get_client(server_t *server, int idx){

}
void server_start(server_t *server, char *server_name, int perms){

}
void server_shutdown(server_t *server){

}
int server_add_client(server_t *server, join_t *join){

}
int server_remove_client(server_t *server, int idx){

}
int server_broadcast(server_t *server, mesg_t *mesg){

}
void server_check_sources(server_t *server){

}
int server_join_ready(server_t *server){

}
int server_handle_join(server_t *server){

}
int server_client_ready(server_t *server, int idx){

}
int server_handle_client(server_t *server, int idx){

}
void server_tick(server_t *server){

}
void server_ping_clients(server_t *server){

}
void server_remove_disconnected(server_t *server, int disconnect_secs){

}
void server_write_who(server_t *server){

}
void server_log_message(server_t *server, mesg_t *mesg){

}
=======
client_t *server_get_client(server_t *server, int idx);
// Gets a pointer to the client_t struct at the given index. If the
// index is beyond n_clients, the behavior of the function is
// unspecified and may cause a program crash.

void server_start(server_t *server, char *server_name, int perms);
// Initializes and starts the server with the given name. A join fifo
// called "server_name.fifo" should be created. Removes any existing
// file of that name prior to creation. Opens the FIFO and stores its
// file descriptor in join_fd._
//
// ADVANCED: create the log file "server_name.log" and write the
// initial empty who_t contents to its beginning. Ensure that the
// log_fd is position for appending to the end of the file. Create the
// POSIX semaphore "/server_name.sem" and initialize it to 1 to
// control access to the who_t portion of the log.

void server_shutdown(server_t *server);
// Shut down the server. Close the join FIFO and unlink (remove) it so
// that no further clients can join. Send a BL_SHUTDOWN message to all
// clients and proceed to remove all clients in any order.
//
// ADVANCED: Close the log file. Close the log semaphore and unlink
// it.

int server_add_client(server_t *server, join_t *join);
// Adds a client to the server according to the parameter join which
// should have fileds such as name filed in.  The client data is
// copied into the client[] array and file descriptors are opened for
// its to-server and to-client FIFOs. Initializes the data_ready field
// for the client to 0. Returns 0 on success and non-zero if the
// server as no space for clients (n_clients == MAXCLIENTS).

int server_remove_client(server_t *server, int idx);
// Remove the given client likely due to its having departed or
// disconnected. Close fifos associated with the client and remove
// them.  Shift the remaining clients to lower indices of the client[]
// array and decrease n_clients.

int server_broadcast(server_t *server, mesg_t *mesg);
// Send the given message to all clients connected to the server by
// writing it to the file descriptors associated with them.
//
// ADVANCED: Log the broadcast message unless it is a PING which
// should not be written to the log.

void server_check_sources(server_t *server);
// Checks all sources of data for the server to determine if any are
// ready for reading. Sets the servers join_ready flag and the
// data_ready flags of each of client if data is ready for them.
// Makes use of the select() system call to efficiently determine
// which sources are ready.

int server_join_ready(server_t *server);
// Return the join_ready flag from the server which indicates whether
// a call to server_handle_join() is safe.

int server_handle_join(server_t *server);
// Call this function only if server_join_ready() returns true. Read a
// join request and add the new client to the server. After finishing,
// set the servers join_ready flag to 0.

int server_client_ready(server_t *server, int idx);
// Return the data_ready field of the given client which indicates
// whether the client has data ready to be read from it.

int server_handle_client(server_t *server, int idx);
// Process a message from the specified client. This function should
// only be called if server_client_ready() returns true. Read a
// message from to_server_fd and analyze the message kind. Departure
// and Message types should be broadcast to all other clients.  Ping
// responses should only change the last_contact_time below. Behavior
// for other message types is not specified. Clear the client's
// data_ready flag so it has value 0.
//
// ADVANCED: Update the last_contact_time of the client to the current
// server time_sec.

void server_tick(server_t *server);
// ADVANCED: Increment the time for the server

void server_ping_clients(server_t *server);
// ADVANCED: Ping all clients in the server by broadcasting a ping.

void server_remove_disconnected(server_t *server, int disconnect_secs);
// ADVANCED: Check all clients to see if they have contacted the
// server recently. Any client with a last_contact_time field equal to
// or greater than the parameter disconnect_secs should be
// removed. Broadcast that the client was disconnected to remaining
// clients.  Process clients from lowest to highest and take care of
// loop indexing as clients may be removed during the loop
// necessitating index adjustments.

void server_write_who(server_t *server);
// ADVANCED: Write the current set of clients logged into the server
// to the BEGINNING the log_fd. Ensure that the write is protected by
// locking the semaphore associated with the log file. Since it may
// take some time to complete this operation (acquire semaphore then
// write) it should likely be done in its own thread to preven the
// main server operations from stalling.  For threaded I/O, consider
// using the pwrite() function to write to a specific location in an
// open file descriptor which will not alter the position of log_fd so
// that appends continue to write to the end of the file.

void server_log_message(server_t *server, mesg_t *mesg);
// ADVANCED: Write the given message to the end of log file associated
// with the server.
>>>>>>> e8e1483feaf7fb525844e6feca32e735392473f6
