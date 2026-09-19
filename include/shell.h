#ifndef SHELL_H
#define SHELL_H

#include <signal.h>
#include <limits.h>
#include <stdbool.h>


struct ShellState {
    // Shell's name, taken from argv[0]
    const char* name;

    // Exit code of the last command
    volatile sig_atomic_t exit_code;

    // Working directories
    char cwd[PATH_MAX];
    char oldpwd[PATH_MAX];

    // User info
    const char* home;
    const char* user;

    // Shell interactive mode
    bool interactive;
};


extern struct ShellState shell;

#endif
