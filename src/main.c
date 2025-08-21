#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "trim.h"


int main() {
    char* command = NULL;
    size_t size;
    int exit_code = 0;

    while (1) {
        printf("shell> ");
        ssize_t len = getline(&command, &size, stdin);

        if (len == -1) {
            break;
        }

        // trimming spaces at the start and end of the command
        trim(command);
        if (strlen(command) == 0) continue;


        // tokenize
        char* args[64];
        int i = 0;
        char* token = strtok(command, " ");
        while (token != NULL && i < 63) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;


        // exit command
        if (strcmp(args[0], "exit") == 0) {
            if (args[1] != NULL) {
                char *endptr;
                long val = strtol(args[1], &endptr, 10);

                if (*endptr == '\0') {
                    exit_code = (int)(val % 256);
                    if (exit_code < 0) exit_code += 256;
                } else {
                    fprintf(stderr, "exit: The exit code must be a number.\n");
                    exit_code = 2;
                    exit(exit_code);
                }

                if (args[2] != NULL) {
                    exit_code = 1;
                    fprintf(stderr, "exit: Too many arguments.\n");
                    continue;
                }
            }
            exit(exit_code);
        }
        


        int status;
        pid_t pid = fork();

        if (pid > 0) {
            wait(&status);
            if (WIFEXITED(status)) {
                exit_code = WEXITSTATUS(status);
            }
        }
        else if (pid == 0) {
            execvp(args[0], args);

            perror(args[0]);
            _exit(127);
        }
        else {
            fprintf(stderr, "Fork failed.\n");
        }
    }

    free(command);

    return 0;
}

