#include <stdio.h>
#include <stdlib.h>

#include "commands/exit_builtin.h"
#include "shell.h"


void exit_builtin(char** args) {
    if (args[1] != NULL) {
        char *endptr;
        long val = strtol(args[1], &endptr, 10);

        if (*endptr == '\0') {
            shell.exit_code = (int)(val % 256);
            if (shell.exit_code < 0) shell.exit_code += 256;
        } else {
            fprintf(stderr, "exit: The exit code must be a number.\n");
            shell.exit_code = 2;
            exit(shell.exit_code);
        }

        if (args[2] != NULL) {
            shell.exit_code = 1;
            fprintf(stderr, "exit: Too many arguments.\n");
            return;
        }
    }
    exit(shell.exit_code);
}

