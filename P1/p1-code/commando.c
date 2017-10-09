#include <stdio.h>
#include "commando.h"

int main(int argc, char *argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  /* allocate the cmdctl structure */
  cmdctl_t *ctl = malloc(sizeof (cmdctl_t));										//Allocate the ctl structure
  ctl->size = 0;
  int echo_status = 0;
  if(strcmp(argv[0],"--echo") || getenv("COMMANDO_ECHO") != NULL){							//Checks if echo is enabled
    echo_status = 1;													//If echo is enabled, sets echo indicator to a positive value
  }
  while(1){														//While loop for commando shell
    printf("@> ");													//Prompt symbol
    char str[MAX_LINE];													//Sets character buffer to receive input
    char *success = fgets(str, MAX_LINE, stdin);									//Stores input in str
    if(success == NULL){												//Checks if there is any remaining input
      printf("\nEnd of input");												//Print "End of Input" if no input remains
      break;
    }
    if(echo_status){													//Check if echo is enabled
      printf("%s", str);												//Print command back to user if they have enabled echo
    }
    int ntok;
    char *tokens[ARG_MAX];
    parse_into_tokens(str, tokens, &ntok);										//Split command into separate strings
    if(ntok == 0){}
    else if(strcmp(tokens[0], "help") == 0){										//If command "help" is entered, list possible built-in commands
      printf("COMMANDO COMMANDS\n");								
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
    else if(strcmp(tokens[0], "exit") == 0){										//If command "exit" is entered, commando is exited
      break;
    }
    else if(strcmp(tokens[0], "list") == 0){										//If command "list" is entered, the current jobs are listed
      cmdctl_print(ctl);
    }
    else if(strcmp(tokens[0], "pause") == 0){										//If command "pause" is entered, commando is paused for the number of seconds and milliseconds specified by the user
      pause_for(atoi(tokens[1]),atoi(tokens[2]));
    }
    else if(strcmp(tokens[0], "output-for") == 0){									//If command "output-for" is entered, the output of the job with the user specified number is printed
      cmd_t *cmd = ctl->cmd[atoi(tokens[1])];
      printf("@<<< Output for %s[#%d] (%d bytes):\n", cmd->name, cmd->pid,
        cmd->output_size);
      printf("----------------------------------------\n");
      cmd_print_output(cmd);
      printf("----------------------------------------\n");
    }
    else if(strcmp(tokens[0], "output-all") == 0){									//If command "output-all" is entered, the output of all jobs is printed
      for(int i = 0; i < ctl->size; i++){
        cmd_t *cmd = ctl->cmd[i];
        printf("@<<< Output for %s[#%d] (%d bytes):\n", cmd->name, cmd->pid,
          cmd->output_size);
        printf("----------------------------------------\n");
        cmd_print_output(cmd);
        printf("----------------------------------------\n");
      }
    }
    else if(strcmp(tokens[0], "wait-for") == 0){									//If command "wait-for" is entered, the job with the user specified number is waited for
      cmd_update_state(ctl->cmd[atoi(tokens[1])],DOBLOCK);
    }
    else if(strcmp(tokens[0], "wait-all") == 0){									//If command "wait-all" is entered, all jobs are waited for
      for(int i = 0; i < ctl->size; i++){
        cmd_update_state(ctl->cmd[i],DOBLOCK);
      }
    }
    else{														//If command entered is not built in, then the arguments are analyzed and the requested job is executed and its information is added to the ctl structure
      cmd_t * new_command = cmd_new(tokens);
      cmd_start(new_command);
      cmdctl_add(ctl, new_command);
    }
    //this is the last part of the function, it updates the state of all processes
    cmdctl_update_state(ctl, NOBLOCK);											//The states of the commands are updated
  }
  cmdctl_freeall(ctl);													//Frees the memory of each cmd in the ctl structure
  free(ctl);														//Frees the memory of the actual ctl structure
}
