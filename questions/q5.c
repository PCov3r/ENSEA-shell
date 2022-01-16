#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

/* Functions prototypes */

void print_exit_code(int, struct timespec, struct timespec);

/* Macros and global variables */

#define BUFFER_SIZE 2056
#define PROMPT_SIZE 64
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue sur le shell ENSEA. \nPour quitter, tapez 'exit'.\n";
char prompt[PROMPT_SIZE] = "enseash[]% ";
const char bye_message[PROMPT_SIZE] = "\nBye bye.\n";
const char exit_cmd[] = "exit";

char output_buff[PROMPT_SIZE];

int main(){
	char in_buff[BUFFER_SIZE];
	int pid , status, nb_of_bits;
	struct timespec child_start, child_stop;

	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); // Print welcoming message to terminal
	write(STDOUT_FILENO, prompt, PROMPT_SIZE); // Print prompt message to terminal
	
	while(1) {
		
		nb_of_bits = read(STDIN_FILENO , in_buff,  sizeof(in_buff));
		
		in_buff[nb_of_bits-1] = 0;
		
		if(!strncmp(in_buff, exit_cmd, strlen(in_buff)) || nb_of_bits == 0){ // Compare incoming command with "exit" and check for Ctrl+D (empty command)
			write(STDOUT_FILENO, bye_message, PROMPT_SIZE);
			exit(EXIT_SUCCESS);
		}
		
		if (clock_gettime(CLOCK_MONOTONIC, &child_start) == -1) { //Get child process start
		       perror("Could not acquire clock");
		       exit(EXIT_FAILURE);
		}
		
		pid = fork();
		
		if(pid < 0){
			perror("Could not fork");
			exit(EXIT_FAILURE);
		}
		else if (pid != 0) { // This is the parent process
			wait(&status); // Wait for child to finish
			if (clock_gettime(CLOCK_MONOTONIC, &child_stop) == -1) { //Get child process stop
			       perror("Could not acquire clock");
			       exit(EXIT_FAILURE);
			}
			print_exit_code(status, child_start, child_stop); // Print child exit code
			
		} else { // This is the child process
			execlp(in_buff,in_buff,(char*)NULL); // Execute incoming command
			perror("Could not execute command");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);

}

/* Print command execution informations */

void print_exit_code(int status, struct timespec start, struct timespec stop){
	
	write(STDOUT_FILENO, prompt, 8); // Print first part of prompt message
	
	if (WIFEXITED(status)) { // Save exit code to buffer and print it
		sprintf(output_buff, "exit:%d", WEXITSTATUS(status)); 
		write(STDOUT_FILENO, output_buff, PROMPT_SIZE); 
	}
	else if (WIFSIGNALED(status)) { // Save signal code to buffer and print it
		sprintf(output_buff, "sig:%d", WTERMSIG(status));
		write(STDOUT_FILENO, output_buff, PROMPT_SIZE);
	}
	memset(output_buff, 0, sizeof(output_buff)); // Clear buffer
    if (stop.tv_nsec < start.tv_nsec) {
		stop.tv_nsec += 1.0e9;
		stop.tv_sec--;
	}
	
	if((stop.tv_sec - start.tv_sec) >= 1){ //Write duration of execution in s
		sprintf(output_buff, "|%.2fs", (stop.tv_sec - start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/1.0e9); 
	} else { //Write duration of execution in ms
		sprintf(output_buff, "|%dms", (int) ((stop.tv_nsec-start.tv_nsec)/1.0e6)); 
	}
	
	write(STDOUT_FILENO, output_buff, PROMPT_SIZE);
	write(STDOUT_FILENO, prompt+8, PROMPT_SIZE); // Print last part of prompt message
	
}

