#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>
#include "trim.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


int cd_handle(char**, char*, char*);

int main() {
    char* command = NULL;
    size_t size;
    int exit_code = 0;
    char cwd[PATH_MAX];
    char last_dir[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(1);
    }
    strcpy(last_dir, cwd);


    while (1) {
        printf("shell %s > ", cwd);
        ssize_t len = getline(&command, &size, stdin);

        if (len == -1) {
            exit(1);
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


        // cd command
        if (strcmp(args[0], "cd") == 0) {
            exit_code = cd_handle(args, cwd, last_dir);
            continue;
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


// cd command
int cd_handle(char** args, char* cwd, char* last_dir) {
    char *new_path = malloc(PATH_MAX);
    if (new_path == NULL) {
        perror("cd: malloc fail");
        return 12;
    }
    *new_path = '\0';

    if (args[1] == NULL) { //if no args
        strcpy(new_path, getenv("HOME"));
    } else if (args[2] != NULL) { // too many args
        fprintf(stderr, "Too many arguments for cd.\n");
        free(new_path);
        return 1;
    }


    if (*new_path == '\0' && strcmp(args[1], "-") == 0) { //previous directory
        strcpy(new_path, last_dir);
        printf("%s\n", new_path);
    }

    if (*new_path == '\0')
        strcpy(new_path, args[1]);


    //change dir
    if (chdir(new_path) == -1) {
        perror("cd");
        free(new_path);
        return 2;
    } else {
        strcpy(last_dir, cwd);
        if (getcwd(cwd, PATH_MAX) == NULL) {
            perror("cd");
            return 3;
        }
        free(new_path);
    }

    return 0;
}

