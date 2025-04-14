#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "token.h"
#include "commands.h"

#define MAX_INPUT 1024 
#define PROMPT "mysh> "
#define WELCOME_MESSAGE "Welcome to my shell!\n"
#define EXIT_MESSAGE "mysh: exiting\n"

void processCommand(char *cmdLine) {
    if (strlen(cmdLine) == 0 || cmdLine[0] == '#') {
        return;
    }

    char *tokens[MAX_TOKENS];
    int count = tokenize(cmdLine, tokens);

    command *cmd = parseCommand(tokens, count);

    if (!cmd) {
        fprintf(stderr, "Parsing failed.\n");
    } else {
        if (strcmp(cmd->cmdName, "exit") == 0) {
            printf("mysh: exiting\n");
            freeCommand(cmd);
            for (int i = 0; i < count; i++) {
                free(tokens[i]);
            }
            exit(EXIT_SUCCESS);
        } else if (strcmp(cmd->cmdName, "die") == 0) {
            if (cmd->argument[1]) {
                for (int i = 1; cmd->argument[i]; i++) {
                    printf("%s ", cmd->argument[i]);
                }
                printf("\n");
            }
            freeCommand(cmd);
            for (int i = 0; i < count; i++) {
                free(tokens[i]);
            }
            exit(EXIT_FAILURE);
        }

        //THIS IS FOR TESTING, REMOVE IT AFTERWARDS
        printf("Token count: %d\n", count);
        for (int i = 0; i < count; i++) {
            printf("Token[%d]: %s\n", i, tokens[i]);
        }

        // executeCommand(cmd);
    }

    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    freeCommand(cmd);
}

void readInput(int fd) {
	static char buffer[MAX_INPUT];
    static int buffer_index = 0;
    int bytesRead;

    bytesRead = read(fd, buffer + buffer_index, MAX_INPUT - buffer_index - 1);
    if (bytesRead <= 0) return;

    buffer_index += bytesRead;
    buffer[buffer_index] = '\0';

    char *newline = strchr(buffer, '\n');
    if (newline != NULL) {
        *newline = '\0';

        char cmd[MAX_INPUT];
        strncpy(cmd, buffer, MAX_INPUT);
        processCommand(cmd);

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
        } 
        else {
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
        }
        else {
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