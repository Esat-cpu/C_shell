#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "prompt_build.h"
#include "trim.h"
#include "executor.h"
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


// Set shell's name from argv[0]
static void set_shell_name(const char *argv0) {
    const char *s = strrchr(argv0, '/');
    shell.name = s ? (s + 1) : argv0;
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


int main(int argc, char** argv) {
    (void)argc;
    atexit(clean_exit);
    signal(SIGINT, sigint_handler);
    set_shell_name(argv[0]);

    if (isatty(STDIN_FILENO))
        shell.interactive = true;
    else
        shell.interactive = false;

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
        if (shell.interactive) {
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
                if (feof(stdin)) break;

                perror("getline");
                exit(EXIT_FAILURE);
            }
        }


        // trimming spaces at the start and end of the command
        trim(command);
        if (!command[0] || command[0] == '#') continue;
        else add_history(command);

        char *error_message = NULL;

        ExeResult e = execute_line(command, &error_message);

        if (e) {
            fprintf(stderr, "%s: %s\n", shell.name, error_message);
            free(error_message);
        }
    }

    return 0;
}

