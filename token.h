#ifndef TOKEN_H
#define TOKEN_H

typedef struct command {
    char **argument; //argument list also includes the command name
    char *inputFile; // input redirection file ('<')
    char *outputFile; //outpur redirection file ('>)
    struct command *pipeTo; //right side command if this is part of a pipe
} command;

int tokenize(char *line, char **tokens); //splits line into tokens: words, <, >, |
command *parseCommand(char **tokens, int tokenCount); //builds a command struct from tokens
void expandWildcards(command *cmd); //expands any * token into matching file names
void freeCommand(command *cmd); //frees dynamically allocated memory in command

#endif