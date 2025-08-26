#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>

#include "trim.h"
#include "cd_handle.h"

#define MAX_ARGS 64

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


char* command = NULL;

void clean_exit( void ) {
    if (command) free(command);
}

int main() {
    atexit(clean_exit);
    signal(SIGINT, SIG_IGN);
    int exit_code = 0;
    char cwd[PATH_MAX];
    char last_dir[PATH_MAX];

    char *shell_path = malloc(PATH_MAX);
    ssize_t leng;
    if (shell_path != NULL) {
        leng = readlink("/proc/self/exe", shell_path, PATH_MAX - 1);
        if (leng != -1) {
            shell_path[leng] = '\0';
            setenv("SHELL", shell_path, 1);
        }
    }
    free(shell_path);

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(errno);
    }
    strcpy(last_dir, cwd);


    char* home = getenv("HOME");
    char* user = getenv("USER");
    if (user == NULL) user = "shell";

    while (1) {
        if (isatty(STDIN_FILENO)) {
            int prmpt_size = PATH_MAX + strlen(user) + 10;
            char* prompt = malloc(prmpt_size);
            char prmpt_cwd[PATH_MAX];
            if (strncmp(cwd, home, strlen(home)) == 0)
                snprintf(prmpt_cwd, PATH_MAX, "~%s", cwd + strlen(home));
            else
                strcpy(prmpt_cwd, cwd);
            if (exit_code == 0)
                snprintf(prompt, prmpt_size, "\033[1;32m%s \033[1;34m%s\033[0m> ", user, prmpt_cwd);
            else
                snprintf(prompt, prmpt_size, "\033[1;32m%s \033[1;34m%s \033[1;31m[%d]\033[0m> ", user, prmpt_cwd, exit_code);

            if (command) free(command);
            command = readline(prompt);
            free(prompt);
            if (!command) {
                perror("readline"); exit(errno);
            }
        } else {
            size_t size = 0;
            ssize_t len = getline(&command, &size, stdin);
            if (len == -1) {
                if (errno == ENOTTY) break;
                perror("getline"); exit(errno);
            }
        }


        // trimming spaces at the start and end of the command
        trim(command);
        if (!*command) continue;
        else add_history(command);


        // tokenize
        char* args[MAX_ARGS];
        int index_of_dup[MAX_ARGS];
        int dup_count = 0;
        int i = 0;
        char* token = strtok(command, " ");
        glob_t result;

        while (token != NULL && i < MAX_ARGS - 1) {
            if (token[0] == '$') {
                token = getenv(token + 1);
                if (token == NULL) token = "";
            }
            // if there are glob characters
            if (strchr(token, '*') || strchr(token, '?') || strchr(token, '[')) {
                if (glob(token, 0, NULL, &result) == 0) {
                    for (size_t ind = 0; ind < result.gl_pathc; ind++) {
                        if (i < MAX_ARGS - 1) {
                            index_of_dup[dup_count++] = i;
                            args[i++] = strdup(result.gl_pathv[ind]);
                        }
                        else break;
                    }
                    globfree(&result);
                } else { // Glob failed, use original token
                    args[i++] = token;
                }
            } else {
                args[i++] = token;
            }
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
        int fd, saved_out, file_no, redir = 0; // variables for redirection
        int pipefd[2], saved_stdout, saved_stdin, used_pipe = 0;
        for (int j = 0; j <= i; j++) {
            // if j equals i (end of the command(s)) or the index of an operator like '&&', '||' or '|' (pipe)
            if (j == i ||
                (args[j] != NULL &&
                (strcmp(args[j], "&&") == 0 || strcmp(args[j], "||") == 0 || strcmp(args[j], "|") == 0))) {

                if (used_pipe == 1) {
                    dup2(saved_stdout, STDOUT_FILENO);
                    close(saved_stdout);

                    saved_stdin = dup(STDIN_FILENO);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    used_pipe = -1;
                }

                if (args[j] != NULL && strcmp(args[j], "|") == 0) {
                    pipe(pipefd);
                    saved_stdout = dup(STDOUT_FILENO);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                    used_pipe = 1;
                }

                if (cmd_start < j) {
                    char* current_args[MAX_ARGS];
                    int k;
                    for (k = 0; k < (j - cmd_start); k++) {
                        int index = cmd_start + k;
                        // if >, >>, 2> or 2>> is used
                        if (strcmp(args[index], ">") == 0 ||
                            strcmp(args[index], ">>") == 0 ||
                            strcmp(args[index], "2>") == 0 ||
                            strcmp(args[index], "2>>") == 0) {
                            if (args[index + 1] == NULL ||
                                strcmp(args[index + 1], ">") == 0 ||
                                strcmp(args[index + 1], ">>") == 0) {
                                fprintf(stderr, "Redirect error.\n");
                                exit_code = 2;
                                break;
                            }
                            redir = 1;


                            if (strcmp(args[index], ">") == 0 || strcmp(args[index], ">>") == 0) {
                                file_no = STDOUT_FILENO;
                            } else {
                                file_no = STDERR_FILENO;
                            }

                            saved_out = dup(file_no);
                            if (saved_out < 0) {
                                perror("dup");
                                exit_code = errno;
                                break;
                            }

                            if (strcmp(args[index], ">") == 0 || strcmp(args[index], "2>") == 0) {
                                fd = open(args[index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            } else {
                                fd = open(args[index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                            }

                            dup2(fd, file_no);
                            close(fd);
                            break; // things after that isn't command
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
                            signal(SIGINT, SIG_DFL);
                            execvp(current_args[0], current_args);

                            perror(current_args[0]);
                            _exit(127);
                        }
                        else {
                            fprintf(stderr, "Fork failed.\n");
                            exit_code = 1;
                        }
                    }


                    if (used_pipe == -1) {
                        dup2(saved_stdin, STDIN_FILENO);
                        close(saved_stdin);
                        used_pipe = 0;
                    }

                    // if redirection is used
                    if (redir) {
                        dup2(saved_out, file_no);
                        close(saved_out);
                        redir = 0;
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
        } // end of command chain

        for (int f = 0; f < dup_count; f++) {
            free(args[index_of_dup[f]]);
        }
    }

    return 0;
}

