#include "blather.h"


int main(int argc, char *argv[]){
  char * serv_fname = argv[1];			//Retrieve name info from input
  char * user_name = argv[2];
  char user_fname_tc[MAXPATH];			//Char arrays that will contain the full fifo names
  char user_fname_ts[MAXPATH];

  bool shutdown = false;			//Status on whether or not server has shut down

  bool user;

  int serv_fd;					//File descriptor for server FIFO
  int tc_fd;					//File descriptor for to-client FIFO
  int ts_fd;					//File descriptor for to-server FIFO

  strcpy(user_fname_tc,"");			//Add name info to char arrays
  strcpy(user_fname_ts,"");
  strcat(user_fname_tc,user_name);
  strcat(user_fname_tc,"_to_cl");
  strcat(user_fname_ts,user_name);
  strcat(user_fname_ts,"_to_srv");

  mkfifo(user_fname_tc, S_IRUSR | S_IWUSR);	//Make to-client FIFO
  mkfifo(user_fname_ts, S_IRUSR | S_IWUSR);     //Make to-server FIFO
 

  join_t *join;					//Create join request struct

  snprintf(join->name, MAXPATH, "%s", user_name); //Send user name info to join struct
  snprintf(join->to_client_fname, MAXPATH, "%s", user_fname_tc); //Send to-client name info to join struct
  snprintf(join->to_server_fname, MAXPATH, "%s", user_fname_ts); //Send to-server name info to join struct

  serv_fd = open(serv_fname, O_RDWR); 		//Open the server's FIFO
  tc_fd = open(user_fname_tc, O_RDONLY);
  ts_fd = open(user_fname_ts, O_WRONLY);


  write(serv_fd, join, sizeof(join_t));		//Write join request to server's FIFO

  if (user){					//user thread loop
    while (!eoi){
      mesg_t *mesg_to_s;
      snprintf(mesg_to_s->name, MAXNAME, "%s", user_name); 
      fgets(mesg_to_s->body, MAXLINE, STDIN_FILENO);
      mesg_to_s->kind = BL_MESG;  
      write(ts_fd, mesg_to_s, sizeof(mesg_t));
    }
    
  }
  else{						//server thread loop
    while (!shutdown){
      mesg_t *mesg_to_c;
      read(tc_fd, mesg_to_c, sizeof(mesg_t)); 
      char terminal_mesg[MAXLINE];
      if (mesg_to_c->kind == BL_MESG){
        snprintf(terminal_mesg, MAXLINE, "[%s]: %s", mesg_to_c->name, mesg_to_c->body);
      }
      else if (mesg_to_c->kind == BL_JOINED){
        snprintf(terminal_mesg, MAXLINE, "-- %s JOINED --", mesg_to_c->name);

      }
      else if (mesg_to_c->kind == BL_DEPARTED){
	snprintf(terminal_mesg, MAXLINE, "-- %s DEPARTED --", mesg_to_c->name);

      }
      else if (mesg_to_c->kind == BL_SHUTDOWN){
	snprintf(terminal_mesg, MAXLINE, "!!! sunnyvale is shutting down !!!");
      }
      iprintf(
    }

  }
}
