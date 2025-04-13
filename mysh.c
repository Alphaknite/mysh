#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include "token.h"
// #include "commands.h"

#define MAX_INPUT 1024 
#define PROMPT "mysh> "
#define WELCOME_MESSAGE "Welcome to my shell!\n"
#define EXIT_MESSAGE "mysh: exiting\n"

void processCommand(char *cmd) {
	if (strlen(cmd) == 0 || cmd[0] == '#') return; //skip empty or comment lines

    //ONLY FOR TESTING REMOVE THIS AFTERWARDS
    printf("command: %s\n", cmd);

    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "die") == 0) {
        printf(EXIT_MESSAGE);
        exit(0);
    }

    //todo tokenize, parse, execute command
}

void readInput(int fd) {
	static char buffer[MAX_INPUT];
    static int buffer_index = 0;
    int bytesRead;

    //read() into the buffer
    bytesRead = read(fd, buffer + buffer_index, MAX_INPUT - buffer_index - 1);

    if (bytesRead <= 0) {
        return; //EOF or error
    }

    buffer_index += bytesRead;
    buffer[buffer_index] = '\0';

    //process line if newline is found
    char *newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0';

        //make a copy and shift buffer contents
        char cmd[MAX_INPUT];
        strncpy(cmd, buffer, MAX_INPUT);
        processCommand(cmd);

        //move remaining data to the beginning of the buffer
        int remaining = buffer_index - (newline - buffer + 1);
        memmove(buffer, newline + 1, remaining);
        buffer_index = remaining;
    }
}

//interactive mode
void interactiveMode() {
	printf(WELCOME_MESSAGE);

    while (1) {
        printf(PROMPT);
        fflush(stdout);
        readInput(STDIN_FILENO);
    }
}

//batch mode
void batchMode(int fd) {
	while (1) {
        readInput(fd);
        char temp;
        if (read(fd, &temp, 1) == 0) {
            break;
        } else {
            lseek(fd, -1, SEEK_CUR);
        }
    }
}

int main(int argc, char *argv[]) {
	int isATTY = isatty(STDIN_FILENO);

    if (argc == 1) {
        //no file provided
        if (isATTY) {
            interactiveMode();
        } else {
            batchMode(STDIN_FILENO);
        }
    } else if (argc == 2) {
        //file provided
		int fd = open(argv[1], O_RDONLY);
		if (fd < 0) {
			perror("mysh: cannot open file");
			return EXIT_FAILURE;
		}
		batchMode(fd); 
		close(fd);
    } else {
        fprintf(stderr, "Usage: ./mysh [batch_file]\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}