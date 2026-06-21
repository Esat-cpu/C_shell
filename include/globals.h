#ifndef GLOBALS_H
#define GLOBALS_H

#include <signal.h>

/** Maximum number of tokens/args per command. */
#define MAX_ARGS 64

/** Exit code of the last foreground command (set by SIGCHLD handler). */
extern volatile sig_atomic_t exit_code;

/** Path to the history file. */
extern const char* HIS_FILE;

#endif
