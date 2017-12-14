#include "blather.h"
#include <errno.h>

/*
  Authors: Evan Welch & Jakob Urnes
*/

client_t *server_get_client(server_t *server, int idx){
  return &server->client[idx];
}
void server_start(server_t *server, char *server_name, int perms){
  int ret = snprintf(server->server_name, MAXPATH, "%s", server_name);
  check_fail(ret < 0, 1, "server_name initialization failed\n");
  int rm = remove(server->server_name);
  if(rm < 0){
    fprintf(stderr, "can't remove server name file\n");
  }
  int ret2 = mkfifo(server->server_name, perms);
  check_fail(ret2 < 0, 1, "fifo can't be made\n");
  server->join_fd = open(server->server_name, O_RDWR);
  check_fail(server->join_fd < 0, 1, "join fifo can't be opened\n");
  server->join_ready = 0;
  server->n_clients = 0;
}
void server_shutdown(server_t *server){
  int cl = close(server->join_fd);
  check_fail(cl < 0, 1, "join fifo can't be closed\n");
  int rm = remove(server->server_name);
  check_fail(rm < 0, 1, "server name file can't be removed\n");
  mesg_t msg;
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));
  mesg->kind = BL_SHUTDOWN;
  while(server->n_clients > 0){
    client_t *client = server_get_client(server, server->n_clients-1);
    int ret = write(client->to_client_fd, mesg, sizeof(mesg_t));
    check_fail(ret < 0, 1, "server shutdown write to client failed\n");
    client = NULL;
    server->n_clients--;
  }
}
int server_add_client(server_t *server, join_t *join){
  client_t client_actual;
  client_t *client = &client_actual;
  memset(client, 0, sizeof(client_t));
  int ret1 = snprintf(client->name, MAXPATH, "%s", join->name);
  check_fail(ret1 < 0, 1, "client name initialization failed\n");
  int ret2 = snprintf(client->to_client_fname, MAXPATH, "%s", join->to_client_fname);
  check_fail(ret2 < 0, 1, "to_client_fname initialization failed\n");
  int ret3 = snprintf(client->to_server_fname, MAXPATH, "%s", join->to_server_fname);
  check_fail(ret3 < 0, 1, "to_server_fname initialization failed\n");
  client->to_client_fd = open(client->to_client_fname, O_WRONLY);
  check_fail(client->to_client_fd < 0, 1, "to client fifo can't be opened\n");
  client->to_server_fd = open(client->to_server_fname, O_RDONLY);
  check_fail(client->to_server_fd < 0, 1, "to server fifo can't be opened\n");
  client->data_ready = 0;
  if(server->n_clients == MAXCLIENTS){
    return 1;
  }
  server->client[server->n_clients] = *client;
  server->n_clients++;
  return 0;
}
int server_remove_client(server_t *server, int idx){
  client_t *client = server_get_client(server, idx);
  int cl1 = close(client->to_client_fd);
  check_fail(cl1 < 0, 1, "to client fifo can't be closed\n");
  int cl2 = close(client->to_server_fd);
  check_fail(cl2 < 0, 1, "to server fifo can't be closed\n");
  int rm1 = remove(client->to_client_fname);
  check_fail(rm1 < 0, 1, "to client name file can't be removed\n");
  int rm2 = remove(client->to_server_fname);
  check_fail(rm2 < 0, 1, "to server name file can't be removed\n");
  for(int i = idx+1; i < server->n_clients; i++){
    server->client[i-1] = *server_get_client(server, i);
  }
  server->n_clients--;
  return 0;
}
int server_broadcast(server_t *server, mesg_t *mesg){
  for(int i = 0; i < server->n_clients; i++){
    client_t *client = server_get_client(server, i);
    int ret = write(client->to_client_fd, mesg, sizeof(mesg_t));
    check_fail(ret < 0, 1, "write to client %d failed\n", i);
  }
  return 0;
}
void server_check_sources(server_t *server){
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
  int ret = select(max_fd, &fds, NULL, NULL, NULL);
  if(errno == EINTR){
    fprintf(stderr, "the select for server_check_sources received a signal and exited\n");
    return;
  } else{
      check_fail(ret < 0, 1, "select call failed\n");
      // At least one file descriptor has data ready
      if(FD_ISSET(server->join_fd, &fds)){
        server->join_ready = 1;
      }
      for(int i = 0; i < server->n_clients; i++){
        client_t *client = server_get_client(server, i);
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
  join_t join;
  int ret = read(server->join_fd, &join, sizeof(join_t));
  check_fail(ret < 0, 1, "read for join fifo failed\n");
  server_add_client(server, &join);
  client_t *client = server_get_client(server, server->n_clients-1);
  mesg_t msg;
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));
  int ret2 = snprintf(mesg->name, MAXNAME, "%s", client->name);
  check_fail(ret2 < 0, 1, "message name initialization failed\n");
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
  mesg_t msg;
  mesg_t *mesg = &msg;
  memset(mesg, 0, sizeof(mesg_t));
  client_t *client = server_get_client(server, idx);
  int ret = read(client->to_server_fd, mesg, sizeof(mesg_t));
  check_fail(ret < 0, 1, "read for to server fifo failed\n");
  if(mesg->kind == BL_MESG){
    server_broadcast(server, mesg);
    client->data_ready = 0;
  } else if(mesg->kind == BL_DEPARTED) {
      server_remove_client(server, idx);
      server_broadcast(server, mesg);
  }
  return 0;
}
