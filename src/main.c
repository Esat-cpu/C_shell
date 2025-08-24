#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "trim.h"
#include "cd_handle.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


char* command = NULL;

void clean_exit( void ) {
    if (command) free(command);
}

int main() {
    atexit(clean_exit);
    size_t size;
    int exit_code = 0;
    char cwd[PATH_MAX];
    char last_dir[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(errno);
    }
    strcpy(last_dir, cwd);


    char* user = getenv("USER");
    if (user == NULL) user = "shell";

    while (1) {
        printf("%s %s > ", user, cwd);
        fflush(stdout);
        ssize_t len = getline(&command, &size, stdin);

        if (len == -1) {
            exit(errno);
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
        args[i] = NULL; // i is now end of the command(s)


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

    
        // commnad chain
        int cmd_start = 0;
        int fd, saved_stdout, redir = 0;
        for (int j = 0; j <= i; j++) {
            // if j equals i (end of the command(s)) or the index of an operator like '&&' or '||'
            if (j == i || (args[j] != NULL && (strcmp(args[j], "&&") == 0 || strcmp(args[j], "||") == 0))) {
                if (cmd_start < j) {
                    char* current_args[64];
                    int k;
                    for (k = 0; k < (j - cmd_start); k++) {
                        int index = cmd_start + k;
                        // if > or >> used
                        if (strcmp(args[index], ">") == 0 || strcmp(args[index], ">>") == 0) {
                            if (args[index + 1] == NULL || strcmp(args[index + 1], ">") == 0 || strcmp(args[index + 1], ">>") == 0) {
                                fprintf(stderr, "Redirect error.\n");
                                break;
                            }
                            redir = 1;
                            saved_stdout = dup(STDOUT_FILENO);
                            if (saved_stdout < 0) {
                                perror("dup");
                                break;
                            }
                            if (strcmp(args[index], ">") == 0) {
                                fd = open(args[index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            }
                            if (strcmp(args[index], ">>") == 0) {
                                fd = open(args[index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                            }
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                            break;
                        }
                        current_args[k] = args[index];
                    }
                    current_args[k] = NULL;

                    // cd command
                    if (strcmp(current_args[0], "cd") == 0) {
                        exit_code = cd_handle(current_args, cwd, last_dir);
                    }
                    // pwd command
                    else if (strcmp(current_args[0], "pwd") == 0) {
                        printf("%s\n", cwd);
                        exit_code = 0;
                    }
                    else {
                        // execute command
                        int status;
                        pid_t pid = fork();

                        if (pid > 0) {
                            wait(&status);
                            if (WIFEXITED(status))
                                exit_code = WEXITSTATUS(status);
                        }
                        else if (pid == 0) {
                            execvp(current_args[0], current_args);

                            perror(current_args[0]);
                            _exit(127);
                        }
                        else {
                            fprintf(stderr, "Fork failed.\n");
                            exit_code = 1;
                        }
                    }
                }

                else {
                    fprintf(stderr, "Unexpected operator.\n");
                    exit_code = 1;
                    break;
                }

                if (j < i && args[j] != NULL) {
                    if (strcmp(args[j], "&&") == 0 && exit_code != 0) {
                        break;
                    } else if (strcmp(args[j], "||") == 0 && exit_code == 0) {
                        break;
                    }
                }

                cmd_start = j + 1;
            }
            if (redir) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
                redir = 0;
            }
        }
    }

    free(command);

    return 0;
}

