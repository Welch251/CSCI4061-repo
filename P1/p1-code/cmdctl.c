#include "commando.h"

void cmdctl_add(cmdctl_t *ctl, cmd_t *cmd){
	int curr_size = ctl->size;
	ctl->cmd[curr_size] = cmd;
	ctl->size = curr_size + 1;
}
// Add the given cmd to the ctl structure. update the cmd[] array and
// size field.

void cmdctl_print(cmdctl_t *ctl){
	printf("JOB  #PID      STAT   STR_STAT OUTB COMMAND\n");
	cmd_t *current;
	for (int i = 0; i<ctl->size; i++){
		current = ctl->cmd[i];
		printf("%-4d #%-8d %4d %10s %4d ", i, current->pid, current->status,
			current->str_status, current->output_size);
			int j = 0;
		while(current->argv[j] != NULL){
			printf("%s ", current->argv[j]);
			j++;
		}
		printf("\n");
	}
}
// Print all cmd elements in the given ctl structure.  The format of
// the table is
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 0    #17434       0    EXIT(0) 2239 ls -l -a -F
// 1    #17435       0    EXIT(0) 3936 gcc --help
// 2    #17436      -1        RUN   -1 sleep 2
// 3    #17437       0    EXIT(0)  921 cat Makefile
//
// Widths of the fields and justification are as follows
//
// JOB  #PID      STAT   STR_STAT OUTB COMMAND
// 1234  12345678 1234 1234567890 1234 Remaining
// left  left    right      right rigt left
// int   int       int     string  int string
//
// The final field should be the contents of cmd->argv[] with a space
// between each element of the array.

void cmdctl_update_state(cmdctl_t *ctl, int block){
	cmd_t *current;
	for (int k = 0; k < ctl->size; k++){
		current = ctl->cmd[k];
		cmd_update_state(current,block);
	}
}
// Update each cmd in ctl by calling cmd_update_state() which is also
// passed the block argument (either NOBLOCK or DOBLOCK)

void cmdctl_freeall(cmdctl_t *ctl){
	cmd_t *current;
	for (int k = 0; k < ctl->size; k++){
		current = ctl->cmd[k];
		cmd_free(current);
	}
}
// Call cmd_free() on all of the constituent cmd_t's.
