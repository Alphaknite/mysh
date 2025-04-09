/**
 * @authors Rudra Patel and Aditya Shah
 * @date 4/9/2025
 * @brief myshell
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void readInput() {
	//interactiveMode() and batchMode() will call this function
}

void processCommand() {
	//readInput will call this function
}

//interactive mode
void interactiveMode() {
	printf("Welcome to my shell!\n");
    //while loop (but what's the condition)
}

//batch mode
void batchMode(char *file) {

}

int main(int argc, char *argv[]) {
	int input = isatty(STDIN_FILENO);

	printf("input: %d\n", input);
	printf("argc: %d\n", argc);
	
	if (argc == 1) {
        if (input == 1) {
            printf("interactive mode!\n");
        } else {
            printf("batch mode with no file provided\n");
        }
    } else if (argc == 2) {
        printf("batch mode with file provided\n");
    } else {
        printf("Too many arguments\n");
        return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;		
}