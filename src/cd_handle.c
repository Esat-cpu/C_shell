#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "cd_handle.h"

#ifndef PATH_MAX
const int PATH_MAX = 4096;
#endif


// cd command
int cd_handle(char** args, char* cwd, char* last_dir) {
    char* home = getenv("HOME");
    char *new_path = malloc(PATH_MAX);
    if (new_path == NULL) {
        perror("cd: malloc fail");
        return ENOMEM;
    }
    *new_path = '\0';

    if (args[1] == NULL) { //if no args
        strcpy(new_path, home);
    } else if (args[2] != NULL) { // too many args
        fprintf(stderr, "Too many arguments for cd.\n");
        free(new_path);
        return 1;
    }


    if (*new_path == '\0' && strcmp(args[1], "-") == 0) { //previous directory
        strcpy(new_path, last_dir);
        printf("%s\n", new_path);
    }

    if (*new_path == '\0')
        strcpy(new_path, args[1]);
    if (new_path[0] == '~')
        snprintf(new_path, PATH_MAX, "%s%s", home, args[1] + 1);


    //change dir
    if (chdir(new_path) == -1) {
        perror("cd");
        free(new_path);
        return errno;
    } else {
        free(new_path);
        strcpy(last_dir, cwd);
        if (getcwd(cwd, PATH_MAX) == NULL) {
            perror("cd");
            return errno;
        }
    }

    return 0;
}

