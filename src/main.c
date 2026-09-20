#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "prompt_build.h"
#include "util.h"
#include "executor.h"
#include "shell.h"


// History file, set to NULL if saving history file is not required.
// It will be written to and read from the user's home directory.
static const char* HIS_FILE = ".she_history";

static char* command = NULL;
static volatile sig_atomic_t in_readline = false;


static void clean_exit(void) {
    free(command);

    if (shell.interactive) {
        char history_file[PATH_MAX];

        if (shell.home && HIS_FILE) {
            snprintf(history_file, sizeof(history_file),
                    "%s/%s", shell.home, HIS_FILE);
            write_history(history_file);
        }
    }
}


// Set shell's name from argv[0]
static void set_shell_name(const char *argv0) {
    const char *s = strrchr(argv0, '/');
    shell.name = s ? (s + 1) : argv0;
}


// Clear input and go to the next line
static void sigint_handler(int sig) {
    (void)sig;  // suppress unused warning
    shell.exit_code = 130;
    write(STDOUT_FILENO, "\n", 1);

    char prompt[PATH_MAX];
    prompt_build(prompt, PATH_MAX);

    if (in_readline) {
        rl_replace_line("", 0);
        rl_on_new_line();
        rl_set_prompt(prompt);
        rl_redisplay();
    }
}


// This will be executed on shell's startup
static void setup(char** argv) {
    atexit(clean_exit);
    set_shell_name(argv[0]);

    struct passwd* pw = getpwuid(getuid());

    shell.home = pw->pw_dir;
    shell.user = pw->pw_name;

    if (isatty(STDIN_FILENO)) {
        shell.interactive = true;
        signal(SIGINT, sigint_handler);
    }
    else
        shell.interactive = false;

    // Set working directories
    if (getcwd(shell.cwd, sizeof(shell.cwd)) == NULL) {
        perror("getcwd");
        exit(errno);
    }
    strcpy(shell.oldpwd, shell.cwd);

    // Read history from HIS_FILE
    if (shell.home && HIS_FILE) {
        char history_file[PATH_MAX];
        snprintf(history_file, sizeof(history_file),
                "%s/%s", shell.home, HIS_FILE);
        read_history(history_file);
    }

    setenv("HOME", shell.home, 0);
    setenv("USER", shell.user, 0);
    setenv("SHELL", pw->pw_shell, 0);
}


int main(int argc, char** argv) {
    (void)argc;
    setup(argv);

    while (1) {
        if (shell.interactive) {
            char prompt[PATH_MAX];
            prompt_build(prompt, PATH_MAX);

            free(command);

            in_readline = 1;
            command = readline(prompt);
            in_readline = 0;

            if (!command)
                exit(shell.exit_code);
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

        add_history(command);

        char *error_message = NULL;
        ExeResult e = execute_line(command, &error_message);

        if (e) {
            fprintf(stderr, "%s: %s\n", shell.name, error_message);
            free(error_message);
        }
    }

    return 0;
}
