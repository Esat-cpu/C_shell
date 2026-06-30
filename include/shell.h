#ifndef SHELL_H
#define SHELL_H

#include <signal.h>


struct ShellState {
    volatile sig_atomic_t exit_code;
};


extern struct ShellState shell;

#endif
