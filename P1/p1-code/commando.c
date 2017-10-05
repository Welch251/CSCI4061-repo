#include <stdio.h>
#include "commando.h"

int main(int argc, char *argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  /* allocate the cmdctl structure */
  cmdctl_t *ctl = malloc(sizeof (cmdctl_t));
  ctl->size = 0;
  int echo_status = 0;
  if(strcmp(argv[1],"--echo") || getenv("COMMANDO_ECHO") != NULL){
    echo_status = 1;
  }
  while(1){
    printf("@> ");
    char str[MAX_LINE];
    char *success;
    success = fgets(str, MAX_LINE, stdin);
    if(success == NULL){
      printf("End of input");
      break;
    }
    if(echo_status){
      printf("%s\n", str);
    }
    int *ntok;
    char *tokens[];
    parse_into_tokens(str, tokens, ntok);
    if(strcmp(tokens[0], "help") != 0){
      printf("\nCOMMANDO COMMANDS\n");
      printf("help               : show this message\n");
      printf("exit               : exit the program\n");
      printf("list               : list all jobs that have been started giving information on each\n");
      printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
      printf("output-for int     : print the output for given job number\n");
      printf("output-all         : print output for all jobs\n");
      printf("wait-for int       : wait until the given job number finishes\n");
      printf("wait-all           : wait for all jobs to finish\n");
      printf("command arg1 ...   : non-built-in is run as a job\n");
    }
    else if(strcmp(tokens[0], "exit") != 0){
      break;
    }
    else if(strcmp(tokens[0], "list") != 0){
      cmdctl_print(ctl);
    }
    else if(strcmp(tokens[0], "pause nanos secs") != 0){
      pause_for(atoi(tokens[1]),atoi(tokens[2]));
    }
    else if(strcmp(tokens[0], "output-for int") != 0){
      cmd_fetch_output(ctl->cmd[atoi(tokens[1])]);
    }
    else if(strcmp(tokens[0], "output-all") != 0){
      for (int i = 0; i<ctl->size;i++){
          cmd_fetch_output(ctl->cmd[atoi(tokens[1])]);
      }
    }
    else if(strcmp(tokens[0], "wait-for int") != 0){
       cmd_t *process = ctl->cmd[atoi(tokens[1])];
       cmd_update_state(process,DOBLOCK);
    }
    else if(strcmp(tokens[0], "wait-all") != 0){
       wait(NULL);
    }
    else{
        cmd_t * new_command = cmd_new(tokens);
        cmdctl_add(ctl, new_command);
    }
    //this is the last part of the function, it updates the state of all processes
    cmdctl_update_state(ctl, WNOHANG);
  }
  cmdctl_freeall(ctl);
}
