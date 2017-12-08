#include "blather.h"

client_t *server_get_client(server_t *server, int idx){
  return server->client[idx];
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
