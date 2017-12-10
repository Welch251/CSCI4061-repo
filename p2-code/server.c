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
  server->join_ready = 0;
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
    client_t client = server_get_client(server, server->n_clients-1);
    ret = write(client->to_client_fd,msg,sizeof(mesg_t));
    if(ret < 0){
      printf("server shutdown write to client failed");
      exit(0);
    }
    client = NULL;
    server->n_clients--;
  }
}
int server_add_client(server_t *server, join_t *join){
  client_t *client;
  strncpy(client->name, join->name, MAXPATH);
  strncpy(client->to_client_fname, join->to_client_fname, MAXPATH);
  strncpy(client->to_server_fname, join->to_server_fname, MAXPATH);
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
  client = server_get_client(server, idx);
  close(client->to_client_fd);
  close(client->to_server_fd);
  remove(client->to_client_fname);
  remove(client->to_server_fname);
  for(int i = idx+1, i < server->n_clients, i++){
    server->client[i-1] = server_get_client(server, i);
  }
  server->n_clients--;
}
int server_broadcast(server_t *server, mesg_t *mesg){
  for(int i = 0, i < server->n_clients, i++){
    client_t client = server_get_client(server, i);
    write(client->to_client_fd, mesg, sizeof(mesg_t));
  }
}
void server_check_sources(server_t *server){
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(server->join_fd, &fds);
  for(int i = 0, i < server->n_clients, i++){
    client_t client = server_get_client(server, i);
    FD_SET(client->to_server_fd, &fds);
  }
  n = server_get_client(server, server->n_clients-1)->to_server_fd-1;
  ret = select(n, &fds, NULL, NULL, NULL);
  if(ret < 0){
    printf("the select for server_check_sources failed");
    exit(0);
  } else{
      // At least one file descriptor has data ready
      if(FD_ISSET(server->join_fd, &fds)){
        server->join_ready = 1;
      }
      for(int i = 0, i < server->n_clients, i++){
        client_t client = server->client[i];
        if(FD_ISSET(client->to_server_fd, &fds)){
          client->data_ready = 1;
        }
      }
  }
}
int server_join_ready(server_t *server){
  return server->join_ready;
}
int server_handle_join(server_t *server){
  if(server_join_ready(server)){
    join_t *join;
    read(server->join_fd, join, sizeof(join_t));
    server_add_client(server, join);
    server->join_ready = 0;
  }
}
int server_client_ready(server_t *server, int idx){
  client_t client = server_get_client(server, idx);
  return client->data_ready;
}
int server_handle_client(server_t *server, int idx){
  if(server_client_ready(server, idx)){
    mesg_t *mesg;
    client_t client = server_get_client(server, idx);
    read(client->to_server_fd, mesg, sizeof(mesg_t));
    if(mesg->kind == BL_MESG || mesg->kind == BL_DEPARTED){
      server_broadcast(server, mesg);
    }
  }
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
