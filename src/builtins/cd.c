#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "builtins/cd.h"
#include "util.h"
#include "shell.h"


// cd command
int cd(int argc, char** argv) {
    char new_path[PATH_MAX];

    if (argc == 1) {
        if (!shell.home) {
            print_err("cd", "HOME not set");
            return EXIT_FAILURE;
        }
        else
            snprintf(new_path, PATH_MAX, "%s", shell.home);
    }

    else if (argc > 2) {
        print_err("cd", "too many arguments");
        return EXIT_FAILURE;
    }

    else {
        if (strcmp(argv[1], "-") == 0)
            snprintf(new_path, PATH_MAX, "%s", shell.oldpwd);
        else
            snprintf(new_path, PATH_MAX, "%s", argv[1]);
    }

    //change dir
    int ret = chdir(new_path);

    if (ret == -1) {
        print_err("cd", strerror(errno));
        return EXIT_FAILURE;
    }
    else {
        snprintf(shell.oldpwd, PATH_MAX, "%s", shell.cwd);

        if (getcwd(shell.cwd, PATH_MAX) == NULL) {
            print_err("cd", strerror(errno));
            return EXIT_FAILURE;
        }

        setenv("PWD", shell.cwd, 1);
        setenv("OLDPWD", shell.oldpwd, 1);
    }

    return EXIT_SUCCESS;
}
