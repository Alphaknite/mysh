#ifndef TOKEN_H
#define TOKEN_H

#define MAX_TOKENS 100

typedef struct command {
    char *cmdName; //command name
    char **argument; //argument list also includes the command name
    char *inputFile; //input redirection file ('<')
    char *outputFile; //outpur redirection file ('>)
    struct command *pipeTo; //right side command if this is part of a pipe
    int conditional; //0 for none, 1 for and, 2 for or
} command;

int tokenize(char *line, char **tokens); //splits line into tokens: words, <, >, |
command *parseCommand(char **tokens, int count); //builds a command struct from tokens
int expandWildcard(const char *pattern, char ***argList, int *argIndex); //expands wildcards *.<extension>
void freeCommand(command *cmd); //frees dynamically allocated memory in command

#endif