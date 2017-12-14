#include "blather.h"
#include <errno.h>

client_t *server_get_client(server_t *server, int idx){
  return &server->client[idx];
}
void server_start(server_t *server, char *server_name, int perms){
  snprintf(server->server_name, MAXPATH, "%s", server_name);
  remove(server->server_name);
  int ret = mkfifo(server->server_name, perms);
  if(ret < 0){
    printf("fifo can't be made\n");
    exit(0);
  }
  server->join_fd = open(server->server_name, O_RDWR);
  if(server->join_fd < 0){
    printf("join fifo can't be opened");
    exit(0);
  }
  server->join_ready = 0;
  server->n_clients = 0;
}
void server_shutdown(server_t *server){
  close(server->join_fd);
  remove(server->server_name);
  mesg_t msg;
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));
  mesg->kind = BL_SHUTDOWN;
  while(server->n_clients > 0){
    client_t *client = server_get_client(server, server->n_clients-1);
    int ret = write(client->to_client_fd, mesg, sizeof(mesg_t));
    if(ret < 0){
      printf("server shutdown write to client failed\n");
      exit(0);
    }
    client = NULL;
    server->n_clients--;
  }
}
int server_add_client(server_t *server, join_t *join){
  //dbg_printf("creating client\n");
  //dbg_printf("%s is joining\n", join->name);
  //dbg_printf("%s is to client filename\n", join->to_client_fname);
  client_t client_actual;
  client_t *client = &client_actual;
  memset(client, 0, sizeof(client_t));
  snprintf(client->name, MAXPATH, "%s", join->name);
  snprintf(client->to_client_fname, MAXPATH, "%s", join->to_client_fname);
  snprintf(client->to_server_fname, MAXPATH, "%s", join->to_server_fname);
  client->to_client_fd = open(client->to_client_fname, O_WRONLY);
  if(client->to_client_fd < 0){
    //printf("to client fifo '%s' can't be opened\n",client->to_client_fname);
    exit(0);
  }
  client->to_server_fd = open(client->to_server_fname, O_RDONLY);
  if(client->to_server_fd < 0){
    printf("to server fifo can't be opened\n");
    exit(0);
  }
  client->data_ready = 0;
  if(server->n_clients == MAXCLIENTS){
    return 1;
  }
  server->client[server->n_clients] = *client;
  server->n_clients++;
  return 0;
}
int server_remove_client(server_t *server, int idx){
  //dbg_printf("removing client %d\n", idx);
  client_t *client = server_get_client(server, idx);
  close(client->to_client_fd);
  close(client->to_server_fd);
  remove(client->to_client_fname);
  remove(client->to_server_fname);
  for(int i = idx+1; i < server->n_clients; i++){
    //dbg_printf("shifting client %d\n", i);
    server->client[i-1] = *server_get_client(server, i);
  }
  server->n_clients--;
  return 0;
}
int server_broadcast(server_t *server, mesg_t *mesg){
  for(int i = 0; i < server->n_clients; i++){
    client_t *client = server_get_client(server, i);
    //dbg_printf("broadcasting for client %d\n", i);
    int ret = write(client->to_client_fd, mesg, sizeof(mesg_t));
    if(ret < 0){
      printf("write failed\n");
      exit(0);
    }
  }
  return 0;
}
void server_check_sources(server_t *server){
  //dbg_printf("top\n");
  fd_set fds;
  FD_ZERO(&fds);
  int max_fd = server->join_fd;
  FD_SET(server->join_fd, &fds);
  for(int i = 0; i < server->n_clients; i++){
    client_t *client = server_get_client(server, i);
    FD_SET(client->to_server_fd, &fds);
    if(client->to_server_fd > max_fd){
      max_fd = client->to_server_fd;
    }
  }
  max_fd++;
  dbg_printf("running select\n");
  int ret = select(max_fd, &fds, NULL, NULL, NULL);
  dbg_printf("select finished %d\n", ret);
  if(errno == EINTR){
    printf("the select for server_check_sources received a signal and exited\n");
  } else{
      check_fail(ret == -1, 1, "select call failed\n");
      // At least one file descriptor has data ready
      if(FD_ISSET(server->join_fd, &fds)){
        dbg_printf("join ready\n");
        server->join_ready = 1;
      }
      for(int i = 0; i < server->n_clients; i++){
        client_t *client = server_get_client(server, i);
        if(FD_ISSET(client->to_server_fd, &fds)){
          dbg_printf("client %d has data\n", i);
          client->data_ready = 1;
        }
      }
  }
}
int server_join_ready(server_t *server){
  return server->join_ready;
}
int server_handle_join(server_t *server){
  join_t join;
  int ret = read(server->join_fd, &join, sizeof(join_t));
  if(ret < 0){
    printf("read failed\n");
    exit(0);
  }
  server_add_client(server, &join);
  client_t *client = server_get_client(server, server->n_clients-1);
  mesg_t msg;
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));
  strncpy(mesg->name, client->name, MAXNAME);
  mesg->kind = BL_JOINED;
  server_broadcast(server, mesg);
  server->join_ready = 0;
  return 0;
}
int server_client_ready(server_t *server, int idx){
  client_t *client = server_get_client(server, idx);
  return client->data_ready;
}
int server_handle_client(server_t *server, int idx){
  dbg_printf("handling client %d\n", idx);
  mesg_t msg;
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));
  client_t *client = server_get_client(server, idx);
  int ret = read(client->to_server_fd, mesg, sizeof(mesg_t));
  if(ret < 0){
    printf("read failed\n");
    exit(0);
  }
  if(mesg->kind == BL_MESG){
    server_broadcast(server, mesg);
    client->data_ready = 0;
  } else if(mesg->kind == BL_DEPARTED) {
      server_remove_client(server, idx);
      server_broadcast(server, mesg);
  }
  return 0;
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
