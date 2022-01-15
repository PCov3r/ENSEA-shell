#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 2048
#define PROMPT_SIZE 64
const char message_bienvenue[BUFFER_SIZE] = "Bienvenue sur le shell ENSEA. \nPour quitter, tapez 'exit'.\n";
const char prompt[PROMPT_SIZE] = "enseash % ";

int main(){
	write(STDOUT_FILENO, message_bienvenue, BIENVENUE_SIZE); //Print welcoming message
	write(STDOUT_FILENO, prompt, PROMPT_SIZE); //Print prompt message
}
