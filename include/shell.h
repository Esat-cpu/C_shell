#ifndef SHELL_H
#define SHELL_H

#include <signal.h>
#include <limits.h>
#include <stdbool.h>


struct ShellState {
    const char* name;
    volatile sig_atomic_t exit_code;
    char cwd[PATH_MAX];
    char oldpwd[PATH_MAX];
    bool interactive;
};


extern struct ShellState shell;

#endif
