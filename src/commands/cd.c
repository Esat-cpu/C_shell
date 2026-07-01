#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "commands/cd.h"
#include "shell.h"


// cd command
int cd(char** args) {
    char* home = getenv("HOME");
    char *new_path = malloc(PATH_MAX);

    if (new_path == NULL) {
        fprintf(stderr, "cd: allocation failed");
        return 1;
    }

    // make it an empty string
    *new_path = '\0';

    if (args[1] == NULL) { //if no args
        if (home == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            free(new_path);
            return 1;
        }
        strcpy(new_path, home); // set new_path to home dir
    }
    else if (args[2] != NULL) { // too many args
        fprintf(stderr, "cd: too many arguments\n");
        free(new_path);
        return 1;
    }


    if (*new_path == '\0' && args[1] && strcmp(args[1], "-") == 0) { //previous directory
        strcpy(new_path, shell.oldpwd); // set new_path to shell.oldpwd
        printf("%s\n", new_path);
    }

    // if new_path is still empty set it to first argument
    if (*new_path == '\0')
        strcpy(new_path, args[1]);

    if (new_path[0] == '~') {
        if (home == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            free(new_path);
            return 1;
        }
        snprintf(new_path, PATH_MAX, "%s%s", home, args[1] + 1);
    }


    //change dir
    int ret = chdir(new_path);
    free(new_path);

    if (ret == -1) {
        perror("cd");
        return 1;
    }
    else {
        strcpy(shell.oldpwd, shell.cwd);

        if (getcwd(shell.cwd, PATH_MAX) == NULL) {
            perror("cd");
            return 1;
        }

        setenv("PWD", shell.cwd, 1);
        setenv("OLDPWD", shell.oldpwd, 1);
    }

    return 0;
}

