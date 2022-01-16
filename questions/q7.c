#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

/* Functions prototypes */

void add_exit_code(int, struct timespec, struct timespec);
void free_args(char**, int*);
char** parse_cmd(char*, int*);
void execute_cmd(char**, int*);

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
	int cmd_size;
	char** cmd;

	
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
			cmd = parse_cmd(in_buff, &cmd_size);
			
			if (clock_gettime(CLOCK_MONOTONIC, &child_start) == -1) { //Get child process start
               perror("clock_gettime");
               exit(EXIT_FAILURE);
			}
			
			pid = fork(); // Fork to execute command using child
			
			if(pid < 0){
				perror("Could not fork");
				exit(EXIT_FAILURE);
			}
			else if (pid != 0) { // This is the parent process
				wait(&status); // Wait for child to finish
				if (clock_gettime(CLOCK_MONOTONIC, &child_stop) == -1) { //Get child process stop
				   perror("clock_gettime");
				   exit(EXIT_FAILURE);
				}
				add_exit_code(status, child_start, child_stop); // Print child exit code
				free_args(cmd, &cmd_size); // Free command buffer
				
			} else { // This is the child process
				execute_cmd(cmd, &cmd_size); // Execute incoming command
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

/* Print command execution informations */

void add_exit_code(int status, struct timespec start, struct timespec stop){
	
	char exit_code_buff[PROMPT_SIZE];
	
	if (WIFEXITED(status)) { // Save exit code to temp buffer and add it to output buffer
		sprintf(exit_code_buff, "exit:%d", WEXITSTATUS(status)); 
	}
	else if (WIFSIGNALED(status)) { // Save signal code to temp buffer and add it to output buffer
		sprintf(exit_code_buff, "sig:%d", WTERMSIG(status));
	}
	strcat(output_buff, exit_code_buff);
	
    if (stop.tv_nsec < start.tv_nsec) {
		stop.tv_nsec += 1.0e9;
		stop.tv_sec--;
	}
	
	if((stop.tv_sec - start.tv_sec) >= 1){ //Write duration of execution in s
		sprintf(exit_code_buff, "|%.2fs", (stop.tv_sec - start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/1.0e9); 
	} else { //Write duration of execution in ms
		sprintf(exit_code_buff, "|%dms", (int) ((stop.tv_nsec-start.tv_nsec)/1.0e6)); 
	}
	strcat(output_buff, exit_code_buff);
	
}

/* Parse the command into words */

char** parse_cmd(char* input, int* cmd_size){
	
	char** result    = 0;
	char * token ;
	int count = 0;
	int i, idx;
	int length = strlen(input);
	
	for(i=0;i<length;i++){ // Count number of words by counting number of space
    if(input[i] == ' ')
        count++;
	}

	result = malloc((count+1)*sizeof(char*)); // Number of words = nb of space + 1
	if(result==NULL) { 
		perror("Malloc did not succeed");
		exit(EXIT_FAILURE);
	}
	
	token = strtok(input, " "); // Search for first word in incoming command
	
	idx = 0;
	while(token != NULL){ // While there is still words
		result[idx] = malloc(strlen(token));
		strcpy(result[idx], token);
		idx++;
		token = strtok(NULL, " "); // Search for next word
   }
   
   *cmd_size = count+1;
   return result;

}

/* Free all the memory used for the command parsing */

void free_args(char **args, int* nbArgs){
	
	int i;
	for(i = 0; i < *nbArgs; i++)
	{
		free(args[i]);
		args[i] = NULL;
	}
	free(args);

	*nbArgs = 0;
}

/* Execute complex command */

void execute_cmd(char **args, int* nbArgs){
	
		int i;
		int redirection_flag = 0; // Will be set to one if a '>' or '<' is detected
		int word_to_erase = 0; // 
		FILE* fichier;
		
		for(i = 0; i < *nbArgs; i++) {
			if(!strcmp(args[i], ">")){ // Redirection to a new output
				redirection_flag = 1;
				fichier = fopen(args[i+1], "w"); // Open the file we want to use as an output
				if (fichier == NULL) {
					perror("Could not open or create file");
					exit(EXIT_FAILURE);
				}
			dup2(fileno(fichier), STDOUT_FILENO); // Redirect shell output to the file
			fclose(fichier);
			}
			
			else if(!strcmp(args[i], "<")){ // Use a file as input
				redirection_flag = 1;
				fichier = fopen(args[i+1], "r"); // Open the file we want to use as an input
				if (fichier == NULL) {
					perror("Could not open file");
					exit(EXIT_FAILURE);
				}
			dup2(fileno(fichier), STDIN_FILENO); // Redirect shell input to the file
			fclose(fichier);
			}
			
			if(redirection_flag){ // Redirection: what comes after '>' or '<' is not needed for the execution
				free(args[i]);
				args[i] = NULL;
				word_to_erase++;
			}
		}
		
		*nbArgs -= word_to_erase; // Update the number of argument
		execvp(args[0], args);
}

