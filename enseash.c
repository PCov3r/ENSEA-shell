#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>

/*Functions prototypes */

int process_input(char*, char***, int*, char***, int*);
int search_pipe(char*, char**);
char **parse_cmd(char*, int*);
void free_args(char**, int*);
void add_exit_code(int, struct timespec, struct timespec);
void simple_cmd(char**, int);
void execute_cmd(char**, int*);
void pipe_cmd(char**, int, char**, int);

/*Macros and global variables */
#define BUFFER_SIZE 2056
#define PROMPT_SIZE 64
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue sur le shell ENSEA. \nPour quitter, tapez 'exit'.\n";
char prompt[PROMPT_SIZE] = "enseash[]% ";
const char bye_message[PROMPT_SIZE] = "\nBye bye.\n";
const char exit_cmd[] = "exit";

char output_buff[PROMPT_SIZE];

int main()
{
	char in_buff[BUFFER_SIZE];
	int nb_of_bits;
	int cmd_left_size;
	int cmd_right_size;
	char **cmd_left;
	char **cmd_right;

	write(STDOUT_FILENO, message_bienvenue, BUFFER_SIZE);	// Print welcoming message to terminal
	write(STDOUT_FILENO, prompt, PROMPT_SIZE);	// Print prompt message to terminal

	while ((nb_of_bits = read(STDIN_FILENO, in_buff, sizeof(in_buff))) > 0)
	{

		strncat(output_buff, prompt, 8);	// Add first part of prompt message

		if (!strncmp(in_buff, exit_cmd, strlen(exit_cmd))) // Compare incoming command with "exit"
		{
			
			write(STDOUT_FILENO, bye_message, PROMPT_SIZE);
			exit(EXIT_SUCCESS);
		}
		else if (nb_of_bits > 1) // Check if there is an incoming command, and not just a '\n'
		{

			in_buff[nb_of_bits - 1] = 0;
			int exec_flag = process_input(in_buff, &cmd_left, &cmd_left_size, &cmd_right, &cmd_right_size); // Type of command to be executed
			if (exec_flag == 1) // Pipe commande
			{
				pipe_cmd(cmd_left, cmd_left_size, cmd_right, cmd_right_size);
			}
			else // Simple command
			{
				simple_cmd(cmd_left, cmd_left_size);
			}
		}
		strcat(output_buff, prompt + 8);	// Add last part of prompt message
		write(STDOUT_FILENO, output_buff, PROMPT_SIZE);	// Print prompt message
		memset(output_buff, 0, sizeof(output_buff));	// Clear buffer

	}

	write(STDOUT_FILENO, bye_message, PROMPT_SIZE);	//If we get there, we received a Ctrl+D
	exit(EXIT_SUCCESS);

}

/* Get type of incoming cmd : pipe or not */

int process_input(char *input, char ***parsed_cmd_left, int *left_cmd_size, char ***parsed_cmd_right, int *right_cmd_size)
{

	int is_pipe = 0;
	char *result[2];
	is_pipe = search_pipe(input, result); // Search for a pipe

	if (is_pipe) // Pipe found, input has been seaparated in half
	{
		*parsed_cmd_left = parse_cmd(result[0], left_cmd_size); // Parse first half
		*parsed_cmd_right = parse_cmd(result[1], right_cmd_size); // Parse second half
	}
	else
	{
		*parsed_cmd_left = parse_cmd(input, left_cmd_size); // No pipe found, parse whole command
	}

	return is_pipe;

}

/* Search for a pipe character : | */

int search_pipe(char *input, char **result)
{

	char *token;

	token = strsep(&input, "|"); // Search for | in input

	result[0] = token; // Part before the |
	result[1] = input; // Part after the |

	if (result[1] == NULL) // No pipe character found
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/* Parse string into words array */

char **parse_cmd(char *input, int *cmd_size)
{

	char **result = 0;
	char *token;
	int count = 0;
	int i, idx;
	int length = strlen(input);

	for (i = 0; i < length; i++)
	{
		// Count number of words by counting number of space
		if (input[i] == ' ')
			count++;
	}

	result = malloc((count + 1) *sizeof(char*));  // Number of words = nb of space + 1
	if (result == NULL)
	{
		perror("Malloc did not succeed");
		exit(EXIT_FAILURE);
	}

	token = strtok(input, " ");	// Search for first word in incoming command

	idx = 0;
	while (token != NULL) // While there is still words
	{
		result[idx] = malloc(strlen(token));
		strcpy(result[idx], token);
		idx++;
		token = strtok(NULL, " ");	// Search for next word
	}

	*cmd_size = count + 1;
	return result;

}

/*Free all memory used for the command parsing */

void free_args(char **args, int *nbArgs)
{

	int i;
	for (i = 0; i<*nbArgs; i++)
	{
		free(args[i]);
		args[i] = NULL;
	}
	free(args);

	*nbArgs = 0;
}

/*Print command execution informations */

void add_exit_code(int status, struct timespec start, struct timespec stop)
{

	char exit_code_buff[PROMPT_SIZE];

	if (WIFEXITED(status))
	{
		// Save exit code to temp buffer and add it to output buffer
		sprintf(exit_code_buff, "exit:%d", WEXITSTATUS(status));
	}
	else if (WIFSIGNALED(status))
	{
		// Save signal code to temp buffer and add it to output buffer
		sprintf(exit_code_buff, "sig:%d", WTERMSIG(status));
	}
	strcat(output_buff, exit_code_buff);

	if (stop.tv_nsec < start.tv_nsec)
	{
		stop.tv_nsec += 1.0e9;
		stop.tv_sec--;
	}

	if ((stop.tv_sec - start.tv_sec) >= 1)
	{
		//Write duration of execution in s
		sprintf(exit_code_buff, "|%.2fs", (stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec) / 1.0e9);
	}
	else
	{
		//Write duration of execution in ms
		sprintf(exit_code_buff, "|%dms", (int)((stop.tv_nsec - start.tv_nsec) / 1.0e6));
	}
	strcat(output_buff, exit_code_buff);

}

/* Execute command with no pipe needed */

void simple_cmd(char **cmd, int cmd_size)
{
	int pid, status;
	struct timespec child_start, child_stop;

	if (clock_gettime(CLOCK_MONOTONIC, &child_start) == -1) //Get child process start
	{
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	pid = fork();	// Fork to execute command using child

	if (pid < 0)
	{
		perror("Could not fork");
		exit(EXIT_FAILURE);
	}
	else if (pid != 0) // This is the parent process
	{
		wait(&status);	// Wait for child to finish
		if (clock_gettime(CLOCK_MONOTONIC, &child_stop) == -1) //Get child process stop
		{
			perror("clock_gettime");
			exit(EXIT_FAILURE);
		}
		add_exit_code(status, child_start, child_stop);	// Print child exit code
		free_args(cmd, &cmd_size);	// Free command buffer

	}
	else // This is the child process
	{
		execute_cmd(cmd, &cmd_size);	// Execute incoming command
		perror("Could not execute command");
		exit(EXIT_FAILURE);
	}
}

/* Search for redirection character aka > or < in incoming args */

void execute_cmd(char **args, int *nbArgs)
{

	int i;
	int redirection_flag = 0;	// Will be set to one if a '>' or '<' is detected
	int word_to_erase = 0;	// 
	FILE * fichier;

	for (i = 0; i<*nbArgs; i++)
	{
		if (!strcmp(args[i], ">")) // Redirection to a new output
		{
			redirection_flag = 1;
			fichier = fopen(args[i + 1], "w");	// Open the file we want to use as an output
			if (fichier == NULL)
			{
				perror("Could not open or create file");
				exit(EXIT_FAILURE);
			}
			dup2(fileno(fichier), STDOUT_FILENO);	// Redirect shell output to the file
			fclose(fichier);
		}
		else if (!strcmp(args[i], "<")) // Use a file as input
		{
			redirection_flag = 1;
			fichier = fopen(args[i + 1], "r");	// Open the file we want to use as an input
			if (fichier == NULL)
			{
				perror("Could not open file");
				exit(EXIT_FAILURE);
			}
			dup2(fileno(fichier), STDIN_FILENO);	// Redirect shell input to the file
			fclose(fichier);
		}

		if (redirection_flag) // Redirection: what comes after '>' or '<' is not needed for the execution
		{
			free(args[i]);
			args[i] = NULL;
			word_to_erase++;
		}
	}

	*nbArgs -= word_to_erase;	// Update the number of argument
	execvp(args[0], args);
}

/* Execute command with pipe | */

void pipe_cmd(char **cmd_left, int cmd_left_size, char **cmd_right, int cmd_right_size)
{
	int pid1, pid2;
	int status;
	struct timespec child_start, child_stop;
	int fd[2];
	pipe(fd);

	if (clock_gettime(CLOCK_MONOTONIC, &child_start) == -1) //Get process start
	{
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	pid1 = fork();	// Fork to execute first half of command using child

	if (pid1 < 0)
	{
		perror("Could not fork");
		exit(EXIT_FAILURE);
	}
	else if (pid1 != 0) // This is the parent process
	{
		pid2 = fork(); // Fork again to execute second half
		if (pid2 < 0)
		{
			perror("Could not fork");
			exit(EXIT_FAILURE);
		}
		else if (pid2 != 0) // This is the parent again
		{
			close(fd[0]); // Close both file descriptors
			close(fd[1]);

			wait(&status); // Wait for children to finish
			wait(NULL);	
			if (clock_gettime(CLOCK_MONOTONIC, &child_stop) == -1) //Get process end time
			{
				perror("clock_gettime");
				exit(EXIT_FAILURE);
			}
			add_exit_code(status, child_start, child_stop);	// Print child exit code
			free_args(cmd_right, &cmd_right_size);	// Free command buffer
			free_args(cmd_left, &cmd_left_size);
		}
		else
		{
			close(fd[0]); // the child does not need the input of the pipe
			int ret = dup2(fd[1], STDOUT_FILENO); // Redirect output to pipe output
			if (ret < 0) perror("dup2 failed:");
			execvp(cmd_left[0], cmd_left);
		}
	}
	else // This is the child process
	{
		close(fd[1]); // the child does not need the output of the pipe
		int ret = dup2(fd[0], STDIN_FILENO); // Redirect input to pipe input
		if (ret < 0) perror("dup2 failed:");
		execvp(cmd_right[0], cmd_right);
		perror("Could not execute command");
		exit(EXIT_FAILURE);
	}
}

