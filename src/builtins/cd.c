#include <stdlib.h>
#include <unistd.h>
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
            strncpy(new_path, shell.home, PATH_MAX);
    }

    else if (argc > 2) {
        print_err("cd", "too many arguments");
        return EXIT_FAILURE;
    }

    else {
        if (strcmp(argv[1], "-") == 0)
            strncpy(new_path, shell.oldpwd, PATH_MAX);
        else
            strncpy(new_path, argv[1], PATH_MAX);
    }

    //change dir
    int ret = chdir(new_path);

    if (ret == -1) {
        print_err("cd", strerror(errno));
        return EXIT_FAILURE;
    }
    else {
        strncpy(shell.oldpwd, shell.cwd, PATH_MAX);

        if (getcwd(shell.cwd, PATH_MAX) == NULL) {
            print_err("cd", strerror(errno));
            return EXIT_FAILURE;
        }

        setenv("PWD", shell.cwd, 1);
        setenv("OLDPWD", shell.oldpwd, 1);
    }

    return EXIT_SUCCESS;
}
