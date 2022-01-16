#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

/* Functions prototypes */

void add_exit_code(int);

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

	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); // Print welcoming message to terminal
	write(STDOUT_FILENO, prompt, PROMPT_SIZE); // Print prompt message to terminal
	
	while((nb_of_bits = read(STDIN_FILENO , in_buff,  sizeof(in_buff))) > 0) {
		
		strncat(output_buff, prompt, 8); // Add first part of prompt message
		
		if(!strncmp(in_buff, exit_cmd, strlen(exit_cmd))){ // Compare incoming command with "exit"
			write(STDOUT_FILENO, bye_message, PROMPT_SIZE);
			exit(EXIT_SUCCESS);
		}
		
		else if(nb_of_bits > 1){ // Check if there is an incoming command, and not just a '\n'
		
			in_buff[nb_of_bits-1] = 0;
			
			pid = fork(); // Fork to execute command using child
			
			if(pid < 0){
				perror("Could not fork");
				exit(EXIT_FAILURE);
			}
			else if (pid != 0) { // This is the parent process
				wait(&status); // Wait for child to finish
				add_exit_code(status); // Print child exit code
				
			} else { // This is the child process
				execlp(in_buff,in_buff,(char*)NULL); // Execute incoming command
				perror("Could not execute command");
				exit(EXIT_FAILURE);
			}
		}
		strcat(output_buff, prompt+8); // Add last part of prompt message
		write(STDOUT_FILENO, output_buff, PROMPT_SIZE); // Print prompt message
		memset(output_buff, 0, sizeof(output_buff)); // Clear buffer
		
	}
	
	write(STDOUT_FILENO, bye_message, PROMPT_SIZE); //If we get there, we received a Ctrl+D
	exit(EXIT_SUCCESS);

}


void add_exit_code(int status){
	
	char child_code_buff[PROMPT_SIZE];
	
	if (WIFEXITED(status)) { // Save exit code to temp buffer and add it to output buffer
		sprintf(child_code_buff, "exit:%d", WEXITSTATUS(status)); 
		strcat(output_buff, child_code_buff); 
	}
	else if (WIFSIGNALED(status)) { // Save signal code to temp buffer and add it to output buffer
		sprintf(child_code_buff, "sig:%d", WTERMSIG(status));
		strcat(output_buff, child_code_buff);
	}
	
}
