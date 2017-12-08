#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commando.h"
// cmd.c

cmd_t *cmd_new(char *argv[]){
  cmd_t *cmd = malloc(sizeof(cmd_t));
  int i = 0;
  while(argv[i] != NULL){
    cmd->argv[i] = strdup(argv[i]);
    i++;
  }
  cmd->argv[i] = NULL;
  snprintf(cmd->name, NAME_MAX, "%s", cmd->argv[0]);
  cmd->name[NAME_MAX] = '\0';
  cmd->pid = -1;
  cmd->out_pipe[0] = -1;
  cmd->out_pipe[1] = -1;
  cmd->finished = 0;
  cmd->status = -1;
  snprintf(cmd->str_status, STATUS_LEN, "INIT");
  cmd->str_status[STATUS_LEN] = '\0';
  cmd->output = NULL;
  cmd->output_size = -1;
  return cmd;
}

void cmd_free(cmd_t *cmd){
  //deallocate strings in argv
  if (cmd->output != NULL){
    free(cmd->output);
  }
  free(cmd);
}

void cmd_start(cmd_t *cmd){
  pipe(cmd->out_pipe);
  snprintf(cmd->str_status, STATUS_LEN, "RUN");
  cmd->pid = fork();
	if (cmd->pid == 0){
		dup2(cmd->out_pipe[PWRITE],PWRITE);
		close(cmd->out_pipe[PREAD]);
		execvp(cmd->name,cmd->argv);
	}
	else{
		dup2(cmd->out_pipe[PREAD],PREAD);
		close(cmd->out_pipe[PWRITE]);
	}
}
void *read_all(int fd, int *nread){
  int b_size = BUFSIZE;
  char *buf = (char *) malloc(b_size+1);
  int total_bytes_read = 0;
  int bytes_read = read(fd, buf, b_size);
  total_bytes_read += bytes_read;
  while(bytes_read > 0){
    if(total_bytes_read == b_size){
      b_size *= 2;
      buf = (char *) realloc(buf, b_size);
    }
    bytes_read = read(fd, buf + total_bytes_read, b_size - total_bytes_read);
    total_bytes_read += bytes_read;
  }
  *nread = total_bytes_read;
  buf[total_bytes_read] = '\0';
  return buf;
}
void cmd_fetch_output(cmd_t *cmd){
  if(!cmd->finished){
    printf("%s[#%d] not finished yet", cmd->name, cmd->pid);
 }
  else{
    cmd->output = read_all(cmd->out_pipe[PREAD], &cmd->output_size);
    close(cmd->out_pipe[PREAD]);
  }
}
void cmd_print_output(cmd_t *cmd){
  if(cmd->output == NULL){
    printf("%s[#%d] has no output yet", cmd->name, cmd->pid);
  }
  else{
    write(STDOUT_FILENO, cmd->output, cmd->output_size);
  }
}
void cmd_update_state(cmd_t *cmd, int nohang){
	if (cmd->finished != 1){
	        int status;
		waitpid(cmd->pid, &status, nohang);
		if (WIFEXITED(status)){
			cmd->finished = 1;
			cmd->status = WEXITSTATUS(status);
			snprintf(cmd->str_status, STATUS_LEN, "EXIT(%d)", cmd->status);
			cmd_fetch_output(cmd);
			printf("@!!! %s[#%d]: %s\n", cmd->name, cmd->pid, cmd->str_status);
		}
}
}
