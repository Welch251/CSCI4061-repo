#include "blather.h"

int signalled = 0;

void sig_handler(int signo) {
  signalled = 1;
}

int main(int argc, char *argv[]){
  char * serv_fname = argv[1];
  server_t *server;
  server_start(server, serv_fname, DEFAULT_PERMS);
  signal(SIGTERM, sig_handler);
  signal(SIGINT, sig_handler);
  while (!signalled){
    server_check_sources(server);
    server_handle_join(server);
    for (int i=0; i < server->n_clients; i++){
      if (server_client_ready(server,i)){
        server_handle_client(server,i);
      }
    }
  }
  server_shutdown(server);
  return 0;
}
