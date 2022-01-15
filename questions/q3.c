#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER_SIZE 2056
#define PROMPT_SIZE 64
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue sur le shell ENSEA. \nPour quitter, tapez 'exit'.\n";
const char prompt[PROMPT_SIZE] = "enseash % ";
const char bye_message[PROMPT_SIZE] = "\nBye bye.\n";
const char exit_cmd[] = "exit";

int main(){
	char in_out_buff[BUFFER_SIZE];
	int pid , status, bit_number;

	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); // Print welcoming message to terminal
	
	while(1) {
		write(STDOUT_FILENO, prompt, PROMPT_SIZE); // Print prompt message to terminal
		
		bit_number = read(STDIN_FILENO , in_out_buff,  sizeof(in_out_buff));
		
		if(!strncmp(in_out_buff, exit_cmd, strlen(exit_cmd)) || bit_number == 0){ // Compare incoming command with "exit" and check for Ctrl+D (empty command)
			write(STDOUT_FILENO, bye_message, PROMPT_SIZE);
			exit(EXIT_SUCCESS);
		}
		
		in_out_buff[bit_number-1] = 0;
		
		pid = fork();
		
		if(pid < 0){
			perror("Could not fork\n");
			exit(EXIT_FAILURE);
		}
		else if (pid != 0) { // This is the parent process
			wait(&status); // Wait for child to finish
			
		} else { // This is the child process
			execlp(in_out_buff,in_out_buff,(char*)NULL); // Execute incoming command
			perror("Could not execute command\n");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);

}


