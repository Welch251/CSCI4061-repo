#include "commando.h"

void cmdctl_add(cmdctl_t *ctl, cmd_t *cmd){
	ctl->cmd[ctl->size] = cmd;
	ctl->size += 1;
}
// Add the given cmd to the ctl structure. update the cmd[] array and
// size field.

void cmdctl_print(cmdctl_t *ctl){
	printf("JOB  #PID      STAT   STR_STAT OUTB COMMAND\n"); 			//initial listing of each component presented in a cmd
	cmd_t *current;
	for (int i = 0; i<ctl->size; i++){						//retrieve and list information for each cmd
		current = ctl->cmd[i];
		printf("%-4d #%-8d %4d %10s %4d ", i, current->pid, current->status,	//print the cmd's elements in the correct format
			current->str_status, current->output_size);
			int j = 0;
		while(current->argv[j] != NULL){					//retrieve the rest of the cmd's arguments
			printf("%s ", current->argv[j]);
			j++;
		}
		printf("\n");
	}
}

void cmdctl_update_state(cmdctl_t *ctl, int block){
	cmd_t *current;
	for (int k = 0; k < ctl->size; k++){						//iterate through each cmd in the ctl structure
		current = ctl->cmd[k];
		cmd_update_state(current,block);					//update the state of the selected cmd
	}
}

void cmdctl_freeall(cmdctl_t *ctl){
	cmd_t *current;
	for (int k = 0; k < ctl->size; k++){						//iterate through each cmd in the ctl structure
		current = ctl->cmd[k];
		cmd_free(current);							//free the memory of the selected cmd
	}
}
