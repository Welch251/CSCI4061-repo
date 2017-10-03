#include <stdio.h>

int main(){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  char *built_i;
  while(1){
    printf("@> ");
    char str[MAX_LINE];
    char *success;
    success = fgets(str, MAX_LINE, STDIN);
    if(success == NULL){
      printf("End of input");
      break;
    }
    if /*test argv[1] || COMMANDO_ECHO != NULL */  {
      print("%s", *str)
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
      cmdctl_print(/*ctl */);
    }
    else if(strcmp(tokens[0], "pause") != 0){
      pause_for(tokens[1],tokens[2]);
    }
    else if(strcmp(tokens[0], "output-for") != 0){
      cmd_fetch_output(cmdctl
    }


    //this is the last part of the function, it updates the state of all processes
    cmdctl_update_state(/*ctl, block */);
  }
  cmdctl_freeall(/* ctl */);
}
