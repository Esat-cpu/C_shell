#ifndef GLOBALS_H
#define GLOBALS_H

#include <signal.h>

#define MAX_ARGS 64

extern volatile sig_atomic_t exit_code;
extern const char* HIS_FILE;

#endif
