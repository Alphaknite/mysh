#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "token.h"
#include <sys/wait.h>

#define MAX_INPUT 1024 
#define PROMPT "mysh> "
#define WELCOME_MESSAGE "Welcome to my shell!\n"
#define EXIT_MESSAGE "mysh: exiting\n"

void executeCommand(char **tokens, int count, command *cmd);

//process a single command line
void processCommand(char *cmdLine) {
    if (strlen(cmdLine) == 0 || cmdLine[0] == '#') {
        return; //ignore empty or comment lines
    }

    char *tokens[MAX_TOKENS];
    int count = tokenize(cmdLine, tokens); //split input line into tokens

    command *cmd = parseCommand(tokens, count); //build command struct

    if (!cmd) {
        fprintf(stderr, "parsing failed.\n");
    } else {
        //handle built-in commands
        if (strcmp(cmd->cmdName, "exit") == 0) {
            printf("mysh: exiting\n");
            freeCommand(cmd);
            for (int i = 0; i < count; i++) free(tokens[i]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(cmd->cmdName, "die") == 0) {
            if (cmd->argument[1]) {
                for (int i = 1; cmd->argument[i]; i++) {
                    printf("%s ", cmd->argument[i]);
                }
                printf("\n");
            }
            freeCommand(cmd);
            for (int i = 0; i < count; i++) free(tokens[i]);
            exit(EXIT_FAILURE);
        }

        //execute regular or piped command
        executeCommand(tokens, count, cmd);
    }

    //free token memory
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    freeCommand(cmd);
}

//read and process input from file descriptor
void readInput(int fd) {
    static char buffer[MAX_INPUT];
    static int buffer_index = 0;
    int bytesRead;

    //read into buffer
    bytesRead = read(fd, buffer + buffer_index, MAX_INPUT - buffer_index - 1);
    if (bytesRead <= 0) return;

    buffer_index += bytesRead;
    buffer[buffer_index] = '\0';

    char *start = buffer;
    char *newline;

    //process each complete line
    while ((newline = strchr(start, '\n')) != NULL) {
        *newline = '\0';

        char cmd[MAX_INPUT];
        strncpy(cmd, start, MAX_INPUT);
        cmd[MAX_INPUT - 1] = '\0';

        processCommand(cmd);

        start = newline + 1;
    }

    //move remaining input to beginning of buffer
    int remaining = buffer + buffer_index - start;
    memmove(buffer, start, remaining);
    buffer_index = remaining;
}

//run a command, with support for built-ins, pipes, and redirection
void executeCommand(char **tokens, int count, command *cmd) {
    char *paths[] = {"/usr/local/bin", "/usr/bin", "/bin"};

    //handle cd
    if (strcmp(tokens[0], "cd") == 0) {
        if (count != 2) {
            fprintf(stderr, "mysh: cd requires 1 argument\n");
            return;
        }
        if (chdir(tokens[1]) != 0) {
            perror("mysh: cd failed");
        }
        return;
    }

    //handle pwd
    if (strcmp(tokens[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
        } else {
            perror("mysh: pwd failed");
        }
        return;
    }

    //handle which
    if (strcmp(tokens[0], "which") == 0) {
        if (count != 2) {
            fprintf(stderr, "mysh: which requires 1 argument\n");
            return;
        }
        char *prog = tokens[1];
        for (int i = 0; i < 3; i++) {
            char path[1030];
            snprintf(path, sizeof(path), "%s/%s", paths[i], prog);
            if (access(path, X_OK) == 0) {
                printf("%s\n", path);
                return;
            }
        }
        fprintf(stderr, "mysh: %s not found\n", prog);
        return;
    }

    //handle piped command
    if (cmd->pipeTo != NULL) {
        int fds[2];
        if (pipe(fds) < 0) {
            perror("pipe failed");
            return;
        }

        //left side of pipe
        pid_t left = fork();
        if (left == 0) {
            dup2(fds[1], STDOUT_FILENO);
            close(fds[0]);
            close(fds[1]);

            if (cmd->inputFile) {
                int in = open(cmd->inputFile, O_RDONLY);
                if (in < 0) {
                    perror("mysh: input failed");
                    exit(1);
                }
                dup2(in, STDIN_FILENO);
                close(in);
            }

            if (strchr(cmd->cmdName, '/')) {
                execv(cmd->cmdName, cmd->argument);
            } else {
                char path[1030];
                for (int i = 0; i < 3; i++) {
                    snprintf(path, sizeof(path), "%s/%s", paths[i], cmd->cmdName);
                    if (access(path, X_OK) == 0) {
                        execv(path, cmd->argument);
                    }
                }
            }
            perror("mysh: command not found");
            exit(1);
        }

        //right side of pipe
        pid_t right = fork();
        if (right == 0) {
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
            close(fds[1]);

            if (cmd->pipeTo->outputFile) {
                int out = open(cmd->pipeTo->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                if (out < 0) {
                    perror("mysh: output failed");
                    exit(1);
                }
                dup2(out, STDOUT_FILENO);
                close(out);
            }

            if (strchr(cmd->pipeTo->cmdName, '/')) {
                execv(cmd->pipeTo->cmdName, cmd->pipeTo->argument);
            } else {
                char path[1030];
                for (int i = 0; i < 3; i++) {
                    snprintf(path, sizeof(path), "%s/%s", paths[i], cmd->pipeTo->cmdName);
                    if (access(path, X_OK) == 0) {
                        execv(path, cmd->pipeTo->argument);
                    }
                }
            }
            perror("mysh: command not found");
            exit(1);
        }

        //close pipe and wait for both children
        close(fds[0]);
        close(fds[1]);
        waitpid(left, NULL, 0);
        waitpid(right, NULL, 0);
        return;
    }

    //non-piped external command
    pid_t pid = fork();
    if (pid == 0) {
        if (cmd->inputFile) {
            int input = open(cmd->inputFile, O_RDONLY);
            if (input < 0) {
                perror("mysh: input failed");
                exit(1);
            }
            dup2(input, STDIN_FILENO);
            close(input);
        }

        if (cmd->outputFile) {
            int output = open(cmd->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (output < 0) {
                perror("mysh: output failed");
                exit(1);
            }
            dup2(output, STDOUT_FILENO);
            close(output);
        }

        if (strchr(cmd->cmdName, '/')) {
            execv(cmd->cmdName, cmd->argument);
        } else {
            char path[1030];
            for (int i = 0; i < 3; i++) {
                snprintf(path, sizeof(path), "%s/%s", paths[i], cmd->cmdName);
                if (access(path, X_OK) == 0) {
                    execv(path, cmd->argument);
                }
            }
        }

        perror("mysh: command not found");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("mysh: fork failed");
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
        if (isATTY) {
            interactiveMode();
        } else {
            batchMode(STDIN_FILENO);
        }
    } else if (argc == 2) {
		int fd = open(argv[1], O_RDONLY);
		if (fd < 0) {
			perror("mysh: cannot open file");
			return EXIT_FAILURE;
		}
		batchMode(fd); 
		close(fd);
    } else {
        fprintf(stderr, "usage: ./mysh [batch_file]\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}