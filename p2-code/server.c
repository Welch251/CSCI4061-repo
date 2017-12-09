#include "blather.h"

client_t *server_get_client(server_t *server, int idx){
  return server->client[idx];
}
void server_start(server_t *server, char *server_name, int perms){
  snprintf(server->server_name, MAXPATH, "%s", server_name);
  remove(server->server_name);
  ret = mkfifo(server->server_name, perms);
  if(ret < 0){
    printf("fifo can't be made\n");
    exit(0);
  }
  server->join_fd = open(server->server_name, perms);
  if(server->join_fd < 0){
    printf("join fifo can't be opened");
    exit(0);
  }
  server->join_ready = 1;
  server->n_clients = 0;
  sever->client = NULL;
}
void server_shutdown(server_t *server){
  close(server->join_fd);
  remove(server->server_name);
  mesg_t *msg;
  msg->kind = BL_SHUTDOWN;
  msg->name = NULL;
  msg->body = NULL;
  while(server->n_clients > 0){
    ret = write(server->client[server->n_clients-1]->to_client_fd,msg,sizeof(mesg_t));
    if(ret < 0){
      printf("server shutdown write to client failed");
      exit(0);
    }
    server->client[server->n_clients-1] = NULL;
    server->n_clients--;
  }
}
int server_add_client(server_t *server, join_t *join){
  client_t *client;
  snprintf(client->name, MAXPATH, join->name);
  snprintf(client->to_client_fname, MAXPATH, join->to_client_fname);
  snprintf(client->to_server_fname, MAXPATH, join->to_server_fname);
  client->to_client_fd = open(client->to_client_fname, O_WRONLY);
  if(client->to_client_fd < 0){
    printf("to client fifo can't be opened");
    exit(0);
  }
  client->to_server_fd = open(client->to_server_fname, O_RONLY);
  if(client->to_server_fd < 0){
    printf("to server fifo can't be opened");
    exit(0);
  }
  client->data_ready = 0;
  if(server->n_clients == MAXCLIENTS){
    return 1;
  }
  server->client[server->n_clients] = client;
  server->n_clients++;
  return 0;
}
int server_remove_client(server_t *server, int idx){
  close(server->client[idx]->to_client_fd);
  close(server->client[idx]->to_server_fd);
  remove(server->client[idx]->to_client_fname);
  remove(server->client[idx]->to_server_fname);
  for(int i = idx+1, i < server->n_clients, i++){
    server->client[i-1] = server->client[i];
  }
  server->n_clients--;
}
int server_broadcast(server_t *server, mesg_t *mesg){
  for(int i = 0, i < server->n_clients, i++){
    write(server->client[i]->to_client_fd, mesg, sizeof(mesg_t));
  }
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
