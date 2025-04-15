#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"

int tokenize(char *line, char **tokens) {
    int count = 0;
    char *token = strtok(line, " \t");
    
    while (token != NULL && count < MAX_TOKENS) {
        tokens[count] = strdup(token);
        count++;
        token = strtok(NULL, " \t");
    }

    return count;
}

command *parseCommand(char **tokens, int count) {
    if (count == 0) return NULL;

    command *cmd = calloc(1, sizeof(command));
    int i = 0, argIndex = 0;

    //check for conditional
    if (strcmp(tokens[0], "and") == 0) {
        cmd->conditional = 1;
        i++;
    } else if (strcmp(tokens[0], "or") == 0) {
        cmd->conditional = 2;
        i++;
    }

    //check for pipe
    int pipeIndex = -1;
    for (int j = i; j < count; j++) {
        if (strcmp(tokens[j], "|") == 0) {
            pipeIndex = j;
            break;
        }
    }

    if (pipeIndex != -1) {
        char **rightTokens = &tokens[pipeIndex + 1];
        int rightCount = count - pipeIndex - 1;
        cmd->pipeTo = parseCommand(rightTokens, rightCount);
        count = pipeIndex;  
    }

    //init argument list (max size = count + 1)
    cmd->argument = calloc(count + 1, sizeof(char*));

    for (; i < count; i++) {
        if (strcmp(tokens[i], "<") == 0 && i + 1 < count) {
            cmd->inputFile = strdup(tokens[++i]);
        } else if (strcmp(tokens[i], ">") == 0 && i + 1 < count) {
            cmd->outputFile = strdup(tokens[++i]);
        } else {
            //first real token is the command name
            if (!cmd->cmdName) cmd->cmdName = strdup(tokens[i]);
            cmd->argument[argIndex++] = strdup(tokens[i]);
        }
    }
    cmd->argument[argIndex] = NULL;
    return cmd;
}

void freeCommand(command *cmd) {
    if (!cmd) {
        return;
    } 

    if (cmd->cmdName) { 
        free(cmd->cmdName);
    }

    if (cmd->argument) {
        for (int i = 0; cmd->argument[i]; i++) {
            free(cmd->argument[i]);
        }
        free(cmd->argument);
    }

    if (cmd->inputFile) {
        free(cmd->inputFile);
    } 
        
    if (cmd->outputFile) {
        free(cmd->outputFile);
    }
    if (cmd->pipeTo) { 
        freeCommand(cmd->pipeTo);
    }

    free(cmd);
}