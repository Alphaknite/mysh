#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <fnmatch.h>
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

    cmd->argument = calloc(MAX_TOKENS + 1, sizeof(char*));
    
    for (; i < count; i++) {
        if (strcmp(tokens[i], "<") == 0 && i + 1 < count) {
            cmd->inputFile = strdup(tokens[++i]);
        } else if (strcmp(tokens[i], ">") == 0 && i + 1 < count) {
            cmd->outputFile = strdup(tokens[++i]);
        } else if (strchr(tokens[i], '*')) {
            int matched = expandWildcard(tokens[i], &cmd->argument, &argIndex);
            if (!matched) {
                // if no match, keep the original token
                cmd->argument[argIndex++] = strdup(tokens[i]);
            }
            // set cmdName if not already set
            if (!cmd->cmdName && cmd->argument[0]) {
                cmd->cmdName = strdup(cmd->argument[0]);
            }
        } else {
            if (!cmd->cmdName) cmd->cmdName = strdup(tokens[i]);
            cmd->argument[argIndex++] = strdup(tokens[i]);
        }
    }
    cmd->argument[argIndex] = NULL;
    return cmd;
}

int expandWildcard(const char *pattern, char ***argList, int *argIndex) {
    DIR *dir = opendir(".");
    if (!dir) {
        perror("opendir failed");
        return 0;
    }

    struct dirent *entry;
    int matched = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (fnmatch(pattern, entry->d_name, 0) == 0) {
            (*argList)[*argIndex] = strdup(entry->d_name);
            (*argIndex)++;
            matched = 1;
        }
    }

    closedir(dir);
    return matched;
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