#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

/* Functions prototypes */

void print_exit_code(int);

/* Macros and global variables */

#define BUFFER_SIZE 2056
#define PROMPT_SIZE 64
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue sur le shell ENSEA. \nPour quitter, tapez 'exit'.\n";
char prompt[PROMPT_SIZE] = "enseash[]% ";
const char bye_message[PROMPT_SIZE] = "\nBye bye.\n";
const char exit_cmd[] = "exit";

char child_code_buff[PROMPT_SIZE];

int main(){
	char in_buff[BUFFER_SIZE];
	int pid , status, nb_of_bits;

	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); // Print welcoming message to terminal
	write(STDOUT_FILENO, prompt, PROMPT_SIZE); // Print prompt message to terminal
	
	while(1) {
		
		nb_of_bits = read(STDIN_FILENO , in_buff,  sizeof(in_buff));
		
		if(!strncmp(in_buff, exit_cmd, strlen(exit_cmd)) || nb_of_bits == 0){ // Compare incoming command with "exit" and check for Ctrl+D (empty command)
			write(STDOUT_FILENO, bye_message, PROMPT_SIZE);
			exit(EXIT_SUCCESS);
		}
		
		in_buff[nb_of_bits-1] = 0;
		
		pid = fork();
		
		if(pid < 0){
			perror("Could not fork\n");
			exit(EXIT_FAILURE);
		}
		else if (pid != 0) { // This is the parent process
			wait(&status); // Wait for child to finish
			print_exit_code(status); // Print child exit code
			
		} else { // This is the child process
			execlp(in_buff,in_buff,(char*)NULL); // Execute incoming command
			perror("Could not execute command\n");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);

}


void print_exit_code(int status){
	
	write(STDOUT_FILENO, prompt, 8); // Print first part of prompt message
	
	if (WIFEXITED(status)) { // Save exit code to buffer and print it
		sprintf(child_code_buff, "exit:%d", WEXITSTATUS(status)); 
		write(STDOUT_FILENO, child_code_buff, PROMPT_SIZE); 
	}
	else if (WIFSIGNALED(status)) { // Save signal code to buffer and print it
		sprintf(child_code_buff, "sig:%d", WTERMSIG(status));
		write(STDOUT_FILENO, child_code_buff, PROMPT_SIZE);
	}
	write(STDOUT_FILENO, prompt+8, PROMPT_SIZE); // Print last part of prompt message
	
}
