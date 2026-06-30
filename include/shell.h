#ifndef SHELL_H
#define SHELL_H

#include <signal.h>
#include <limits.h>


struct ShellState {
    volatile sig_atomic_t exit_code;
    char cwd[PATH_MAX];
    char oldpwd[PATH_MAX];
};


extern struct ShellState shell;

#endif
