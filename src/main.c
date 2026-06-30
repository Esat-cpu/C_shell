#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>

#include "commands/cd.h"
#include "commands/exit_builtin.h"
#include "prompt_build.h"
#include "trim.h"
#include "tokenize.h"
#include "expansion.h"
#include "shell.h"

#define MAX_ARGS 64


// History file, set to NULL if saving history file is not required.
static const char* HIS_FILE = ".shell_history";

static char* command = NULL;


void clean_exit(void) {
    if (command) free(command);
    if (isatty(STDIN_FILENO)) {
        char history_file[PATH_MAX];
        char* home = getenv("HOME");
        if (home && HIS_FILE) {
            snprintf(history_file, sizeof(history_file), "%s/%s", home, HIS_FILE);
            write_history(history_file);
        }
    }
}


// clear input and go to the next line
static void sigint_handler(int sig) {
    (void)sig;  // suppress unused warning
    write(STDOUT_FILENO, "\n", 1);
    rl_replace_line("", 0);
    rl_on_new_line();
    rl_redisplay();
    shell.exit_code = 130;
}


int main() {
    atexit(clean_exit);
    signal(SIGINT, sigint_handler);

    // Assign the executable location to the SHELL environment variable
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

    // Get the current directory
    if (getcwd(shell.cwd, sizeof shell.cwd) == NULL) {
        perror("getcwd");
        exit(errno);
    }
    strcpy(shell.oldpwd, shell.cwd);


    // Get the home directory and user name
    char* home = getenv("HOME");
    char* user = getenv("USER");
    if (user == NULL) user = "shell";

    if (home && HIS_FILE) {
        char history_file[PATH_MAX];
        snprintf(history_file, sizeof history_file, "%s/%s", home, HIS_FILE);
        read_history(history_file);
    }

    while (1) {
        if (isatty(STDIN_FILENO)) {
            char prompt[PATH_MAX];
            prompt_build(prompt, PATH_MAX, home, user);


            if (command) free(command);
            command = readline(prompt);

            if (!command) {
                exit(shell.exit_code);
            }
        }
        else {
            size_t size = 0;
            ssize_t len = getline(&command, &size, stdin);

            if (len == -1) {
                if (errno == ENOTTY) break;
                perror("getline");
                exit(EXIT_FAILURE);
            }
        }


        // trimming spaces at the start and end of the command
        trim(command);
        if (!*command) continue;
        else add_history(command);


        // tokenize
        Token argv[MAX_ARGS];
        size_t last_index = tokenize(command, argv, MAX_ARGS);

        // param expansion
        expand_param(argv);

        char* args[MAX_ARGS];
        tokens_to_str_arr(argv, args);



        // exit command
        if (strcmp(args[0], "exit") == 0) {
            exit_builtin(args);
            // If not exited go to next iteration
            continue;
        }


        // commnad chain
        size_t cmd_start = 0;
        int fd, saved_out, file_no, redir = 0; // variables for redirection
        int pipefd[2], saved_stdout, saved_stdin, used_pipe = 0;
        for (size_t j = 0; j <= last_index; j++) {
            // if j equals last_index or the index of an operator like '&&', '||' or '|' (pipe)
            if (j == last_index ||
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
                    for (k = 0; (size_t) k < (j - cmd_start); k++) {
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
                                shell.exit_code = 2;
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
                                shell.exit_code = errno;
                                break;
                            }

                            if (strcmp(args[index], ">") == 0 || strcmp(args[index], "2>") == 0) {
                                fd = open(args[index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            } else {
                                fd = open(args[index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                            }

                            dup2(fd, file_no);
                            close(fd);
                            break; // things after that aren't command
                        }
                        current_args[k] = args[index];
                    }
                    current_args[k] = NULL;

                    // cd command
                    if (strcmp(current_args[0], "cd") == 0) {
                        shell.exit_code = cd(current_args, shell.cwd, shell.oldpwd);
                    }
                    // pwd command
                    else if (strcmp(current_args[0], "pwd") == 0) {
                        printf("%s\n", shell.cwd);
                        shell.exit_code = 0;
                    }
                    else {
                        // execute command
                        int status;
                        pid_t pid = fork();

                        if (pid > 0) {
                            wait(&status);
                            if (WIFEXITED(status))
                                shell.exit_code = WEXITSTATUS(status);
                        }
                        else if (pid == 0) {
                            signal(SIGINT, SIG_DFL);
                            execvp(current_args[0], current_args);

                            perror(current_args[0]);
                            _exit(127);
                        }
                        else {
                            fprintf(stderr, "Fork failed.\n");
                            shell.exit_code = 1;
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
                    shell.exit_code = 1;
                    break;
                }

                if (j < last_index && args[j] != NULL) {
                    if (strcmp(args[j], "&&") == 0 && shell.exit_code != 0) {
                        break;
                    } else if (strcmp(args[j], "||") == 0 && shell.exit_code == 0) {
                        break;
                    }
                }

                cmd_start = j + 1;
            }
        } // end of command chain

        free_tokens(argv);
        memset(args, 0, sizeof args);
    }

    return 0;
}

