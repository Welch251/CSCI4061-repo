#include "blather.h"

int signalled = 0;

void sig_handler(int signo) {
  dbg_printf("handling signal\n");
  signalled = 1;
}

int main(int argc, char *argv[]){
  if(argc != 2){
    printf("usage: %s <server name> \n",argv[0]);
    exit(0);
  }

  char * serv_fname = argv[1];
  server_t server;
  server_t *serv = &server;
  memset(serv, 0, sizeof(server_t));
  server_start(serv, serv_fname, DEFAULT_PERMS);
  signal(SIGTERM, sig_handler);
  signal(SIGINT, sig_handler);
  while (!signalled){
    server_check_sources(serv);
    if(server_join_ready(serv)){
      server_handle_join(serv);
    }
    for (int i=0; i < serv->n_clients; i++){
      if(server_client_ready(serv, i)){
        server_handle_client(serv,i);
      }
    }
  }
  dbg_printf("safe shutdown\n");
  server_shutdown(serv);
  return 0;
}
