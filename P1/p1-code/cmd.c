#include <sctdio.h>
#include <stdlib.h>
#include <string.h>
// cmd.c

cmd_t *cmd_new(char *argv[]){
        cmd_t *cmd = malloc(sizeof (cmd_t));
        cmd->name = strdup(argv[0]);
        cmd->argv = strdup(argv);
        //for (int i = 0;
        //cmd->argv = strdup(argv);
        cmd->pid = NULL;
        cmd->out_pipe = NULL;
        cmd->finished = 0;
        cmd->status = -1;
        cmd->str_status = snprintf("INIT");
        cmd->output = NULL;
        cmd->output_size = -1;
}

void cmd_free(cmd_t *cmd){
        //deallocate strings in argv
        if (cmd->output != NULL){
                free(cmd->output);
        }
        free(cmd);
}
//void cmd_info(cmd_t *cmd);
void cmd_start(cmd_t *cmd){
        cmd->out_pipe = pipe(int pipes[2]);
        cmd->str_status = snprintf("RUN");
        cmd->pid = fork();
	if (cmd->pid == 0){
		dup2(1,pipes[1]);
		close(pipes[0]);
	}
	else{
		dup2(0,pipes[0]);		
		close(pipes[1]);
	}
}
void cmd_fetch_output(cmd_t *cmd);
void cmd_print_output(cmd_t *cmd);
void cmd_update_state(cmd_t *cmd, int nohang){
	if (cmd->finished != 1){
		waitpid(cmd->pid,&status,nohang);
		if (WIFEXITED(status)){
			cmd->finished = 1;
			cmd->status = WEXITSTATUS(status);
			cmd_fetch_output(cmd);
		}	
	}
}
