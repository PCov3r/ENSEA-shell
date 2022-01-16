#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define BUFFER_SIZE 2056
#define PROMPT_SIZE 64
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue sur le shell ENSEA. \nPour quitter, tapez 'exit'.\n";
const char prompt[PROMPT_SIZE] = "enseash % ";

int main(){
	char in_buff[BUFFER_SIZE];
	int pid , status, bit_number;

	
	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE); // Print welcoming message to terminal
	
	while(1) {
		write(STDOUT_FILENO, prompt, PROMPT_SIZE); // Print prompt message to terminal
		
		bit_number = read(STDIN_FILENO , in_buff,  sizeof(in_buff));
		in_buff[bit_number-1] = 0;
		
		pid = fork();
		
		if(pid < 0){
			perror("Could not fork\n");
			exit(EXIT_FAILURE);
		}
		else if (pid != 0) { // This is the parent process
			wait(&status); // Wait for child to finish
			
		} else { // This is the child process
			execlp(in_buff,in_buff,(char*)NULL); // Execute incoming command
			perror("Could not execute command\n");
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);

}

