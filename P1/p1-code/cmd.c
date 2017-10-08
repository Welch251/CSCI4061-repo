#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commando.h"
// cmd.c

cmd_t *cmd_new(char *argv[]){										//Initializes the components of a cmd
  cmd_t *cmd = malloc(sizeof(cmd_t));									//Allocates necessary memory for the struct
  int i = 0;			
  while(argv[i] != NULL){										//Copies each argument of cmd->argv using strdup
    cmd->argv[i] = strdup(argv[i]);
    i++;
  }
  cmd->argv[i] = NULL;
  snprintf(cmd->name, NAME_MAX, "%s", cmd->argv[0]);							//Sets the cmd's name to argv[0]
  cmd->name[NAME_MAX] = '\0';										//Ensure the name is null terminated
  cmd->pid = -1;											//Initializes the pid of the process to -1
  cmd->out_pipe[0] = -1;										//Initializes both pipe elements to -1
  cmd->out_pipe[1] = -1;
  cmd->finished = 0;											//Sets the job finished status to 0, indicating it is not finished
  cmd->status = -1;											//Sets the exit status to -1, as the job has not returned anything yet
  snprintf(cmd->str_status, STATUS_LEN, "INIT");							//Sets status to INIT as the process has been initializes
  cmd->str_status[STATUS_LEN] = '\0';									//Ensure the job status display is null terminated
  cmd->output = NULL;											//Sets output to NULL, as no output has been returned yet
  cmd->output_size = -1;										//Sets output size to -1, as not output has been returned yet
  return cmd;
}

void cmd_free(cmd_t *cmd){
  int i = 0;
  while(cmd->argv[i] != NULL){										//Iterates through each argument of cmd
    free(cmd->argv[i]);											//Frees the argument data
    i++;
  }
  if (cmd->output != NULL){										//Frees the cmd's output if it is not already null
    free(cmd->output);
  }
  free(cmd);												//Frees the remanining components of cmd
}

void cmd_start(cmd_t *cmd){
  pipe(cmd->out_pipe);											//Sets pipe elements as file descriptors
  snprintf(cmd->str_status, STATUS_LEN, "RUN");								//Sets the process status to RUN
  cmd->pid = fork();											//Sets the process as a child
	if (cmd->pid == 0){										//Redirects the child's write output from the screen to the pipe
		dup2(cmd->out_pipe[PWRITE],PWRITE);
		close(cmd->out_pipe[PREAD]);								//Closes the pipe's read input on the child's end 
		execvp(cmd->name,cmd->argv);								//Executes the process
	}
	else{
		dup2(cmd->out_pipe[PREAD],PREAD);							//Redirects the parent's read input from the keyboard 
		close(cmd->out_pipe[PWRITE]);								//Closes the pipe's write output on the parent's end
	}
}
void *read_all(int fd, int *nread){
  int b_size = BUFSIZE;											//Sets a variable to represent the buffer's size in bytes, initially at standard BUFSIZE 1024 bytes
  char *buf = (char *) malloc(b_size+1);								//Allocates the memory of the buffer
  int total_bytes_read = 0;										//Sets a variable to represent the amount of bytes that have been read so far
  int bytes_read = read(fd, buf, b_size);								//Reads the first 1024 bytes from the file
  total_bytes_read += bytes_read;									//Updates the total number of bytes read
  while(bytes_read > 0){										//Loop continues until EOF is reached
    if(total_bytes_read == b_size){									//Checks if the buffer has been filled, statement is entered if it has
      b_size *= 2;											//Doubles the display for the number of bytes, this is then used to indicate the size in the next command
      buf = (char *) realloc(buf, b_size);								//Reallocates a new buffer that is twice the size of the original, still contains the original contents
    }
    bytes_read = read(fd, buf + total_bytes_read, b_size - total_bytes_read);				//Reads the contents of the file into the buffer
    total_bytes_read += bytes_read;									//Updates the total number of bytes read
  }
  *nread = total_bytes_read;										//Saves the total number of bytes read in memory
  buf[total_bytes_read] = '\0';										//Ensures that the buffer is null terminated
  return buf;
}
void cmd_fetch_output(cmd_t *cmd){
  if(!cmd->finished){											//Checks if process has finished
    printf("%s[#%d] not finished yet\n", cmd->name, cmd->pid);						//Prints statement with information stating that it has not finished
  }
  else{													//At this point, the process has finished
    cmd->output = read_all(cmd->out_pipe[PREAD], &cmd->output_size);					//Process output is retrieved using read_all
    close(cmd->out_pipe[PREAD]);									//Closes the read input from the pipe for the parent
  }
}
void cmd_print_output(cmd_t *cmd){									
  if(cmd->output == NULL){										//Checks if process as generated output
    printf("%s[#%d] has no output yet\n", cmd->name, cmd->pid);						//Prints statement with information stating that no output has been generated
  }
  else{													//At this point, the process has generated output
    write(STDOUT_FILENO, cmd->output, cmd->output_size);						//Writes output to cmd->output
  }
}
void cmd_update_state(cmd_t *cmd, int nohang){
	if (cmd->finished != 1){									//Checks if the process has finished
    		int status;										//Initializes location where job status is saved
		if (waitpid(cmd->pid, &status, nohang) > 0){
  		if (WIFEXITED(status)){									//Checks if process has terminated normally
  			cmd->finished = 1;								//Sets indicator that process has finished
  			cmd->status = WEXITSTATUS(status);						//Retrieves the exit status of the process
  			snprintf(cmd->str_status, STATUS_LEN, "EXIT(%d)", cmd->status);			//Sets the process status to EXIT(exit status)
  			cmd_fetch_output(cmd);								//Retrieves the output of the process
  			printf("@!!! %s[#%d]: %s\n", cmd->name, cmd->pid, cmd->str_status);		//A message that indicates what has changed is printed
  		}
    }
	}
}
