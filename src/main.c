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

#include "trim.h"
#include "cd_handle.h"
#include "tokenize.h"
#include "expansion.h"
#include "globals.h"
#include "prompt.h"
#include "redirect.h"

const char* HIS_FILE = ".shell_history";
char* command = NULL;
volatile sig_atomic_t exit_code = 0;


static void
clean_exit(void) {
    if (command) free(command);
    if (isatty(STDIN_FILENO)) {
        char *home = getenv("HOME");
        if (home && HIS_FILE) {
            char path[PATH_MAX];
            snprintf(path, sizeof path, "%s/%s", home, HIS_FILE);
            write_history(path);
        }
    }
}


static void
sigint_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
    rl_replace_line("", 0);
    rl_on_new_line();
    rl_redisplay();
    exit_code = 130;
}


static int
handle_builtin(char **args, char *cwd, char *last_dir) {
    if (strcmp(args[0], "exit") == 0) {
        if (args[1]) {
            char *end;
            long val = strtol(args[1], &end, 10);
            if (*end) {
                fprintf(stderr, "exit: The exit code must be a number.\n");
                exit(2);
            }
            exit_code = (int)(val % 256);
            if (exit_code < 0) exit_code += 256;
            if (args[2]) {
                fprintf(stderr, "exit: Too many arguments.\n");
                exit_code = 1;
                return 1;
            }
        }
        exit(exit_code);
    }

    if (strcmp(args[0], "cd") == 0) {
        exit_code = cd_handle(args, cwd, last_dir);
        return 1;
    }

    if (strcmp(args[0], "pwd") == 0) {
        puts(cwd);
        exit_code = 0;
        return 1;
    }

    return 0;
}


static void
exec_external(char **args) {
    pid_t pid = fork();

    if (pid > 0) {
        int status;
        wait(&status);
        exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }
    else if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        execvp(args[0], args);
        perror(args[0]);
        _exit(127);
    }
    else {
        fprintf(stderr, "Fork failed.\n");
        exit_code = 1;
    }
}


int
main(void) {
    atexit(clean_exit);
    signal(SIGINT, sigint_handler);

    char cwd[PATH_MAX];
    char last_dir[PATH_MAX];
    char *home = getenv("HOME");
    char *user = getenv("USER");
    if (!user) user = "shell";

    if (!getcwd(cwd, sizeof cwd)) { perror("getcwd"); exit(errno); }
    strcpy(last_dir, cwd);

    char *self = malloc(PATH_MAX);
    if (self) {
        ssize_t n = readlink("/proc/self/exe", self, PATH_MAX - 1);
        if (n != -1) { self[n] = '\0'; setenv("SHELL", self, 1); }
        free(self);
    }

    if (home && HIS_FILE) {
        char path[PATH_MAX];
        snprintf(path, sizeof path, "%s/%s", home, HIS_FILE);
        read_history(path);
    }

    while (1) {
        if (isatty(STDIN_FILENO)) {
            int psz = PATH_MAX + strlen(user) + 10;
            char prompt[psz];
            get_prompt(prompt, psz, user, cwd, home, exit_code);

            if (command) free(command);
            command = readline(prompt);
            if (!command) exit(exit_code);
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

        trim(command);
        if (!*command) continue;
        add_history(command);

        Token argv[MAX_ARGS];
        size_t last_index = tokenize(command, argv, MAX_ARGS);

        expand_tilde(argv);
        expand_param(argv, exit_code);

        char *args[MAX_ARGS];
        tokens_to_str_arr(argv, args);

        if (handle_builtin(args, cwd, last_dir)) {
            free_tokens(argv);
            memset(args, 0, sizeof args);
            continue;
        }

        size_t cmd_start = 0;
        int pipefd[2], saved_stdout, saved_stdin, used_pipe = 0;

        for (size_t j = 0; j <= last_index; j++) {
            int is_op = (args[j] != NULL) && (
                strcmp(args[j], "&&") == 0 ||
                strcmp(args[j], "||") == 0 ||
                strcmp(args[j], "|") == 0);

            if (j != last_index && !is_op) continue;

            if (used_pipe == 1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
                saved_stdin = dup(STDIN_FILENO);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                used_pipe = -1;
            }

            if (args[j] && strcmp(args[j], "|") == 0) {
                pipe(pipefd);
                saved_stdout = dup(STDOUT_FILENO);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                used_pipe = 1;
            }

            if (cmd_start == j) {
                fprintf(stderr, "Unexpected operator.\n");
                exit_code = 1;
                if (is_op) { cmd_start = j + 1; continue; }
                break;
            }

            char *segment[MAX_ARGS];
            int seg_len = 0;
            RedirState redir = {{-1, -1}, {0, 0}};
            int err = 0;

            for (int k = 0; (size_t)k < (j - cmd_start);) {
                char *tok = args[cmd_start + k];
                char *next = ((size_t)(k + 1) < (j - cmd_start))
                             ? args[cmd_start + k + 1] : NULL;

                RedirMatch m = match_redirect(tok, next);
                if (m.type != REDIR_NONE) {
                    if (m.error) {
                        fprintf(stderr, "Redirect error.\n");
                        exit_code = 2; err = 1; break;
                    }
                    if (m.type == REDIR_FILE)
                        redir_apply_file(&redir, m.src_fd, m.filename, m.append);
                    else
                        redir_apply_dup(&redir, m.src_fd, m.target_fd);
                    k += m.consumed;
                    continue;
                }

                segment[seg_len++] = tok;
                k++;
            }
            segment[seg_len] = NULL;

            if (!err && seg_len > 0)
                exec_external(segment);

            if (used_pipe == -1) {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
                used_pipe = 0;
            }

            redir_restore(&redir);

            if (is_op && j < last_index) {
                if ((strcmp(args[j], "&&") == 0 && exit_code != 0) ||
                    (strcmp(args[j], "||") == 0 && exit_code == 0))
                    break;
            }

            cmd_start = j + 1;
        }

        free_tokens(argv);
        memset(args, 0, sizeof args);
    }

    return 0;
}
