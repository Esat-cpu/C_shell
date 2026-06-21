#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "cd_handle.h"


int
cd_handle(char** args, char* cwd, char* last_dir) {
    /* args[0]="cd".  args[1] is the target (or NULL for $HOME). */
    char* home = getenv("HOME");
    char* path = NULL;

    if (args[1] == NULL) {
        if (!home) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
        path = home;
    }
    else if (args[2] != NULL) {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }
    else if (strcmp(args[1], "-") == 0) {
        if (!last_dir || !*last_dir) {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return 1;
        }
        path = last_dir;
        puts(path);
    }
    else {
        path = args[1];
    }

    if (chdir(path) == -1) {
        perror("cd");
        return 1;
    }

    if (last_dir) strcpy(last_dir, cwd);

    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("cd");
        return 1;
    }

    setenv("OLDPWD", last_dir, 1);
    setenv("PWD", cwd, 1);

    return 0;
}
