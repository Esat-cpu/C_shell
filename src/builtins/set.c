#include <string.h>
#include <stdbool.h>

#include "builtins/set.h"
#include "shell.h"


int set(char** argv) {
    if (strcmp(argv[1], "-e") == 0)
        shell.errexit = true;
    else if (strcmp(argv[1], "+e") == 0)
        shell.errexit = false;

    return 0;
}
