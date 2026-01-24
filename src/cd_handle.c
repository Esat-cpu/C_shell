#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>


// cd command
int cd_handle(char** args, char* cwd, char* last_dir) {
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
        if (!last_dir || *last_dir == '\0') {
            fprintf(stderr, "cd: last directory not set");
            free(new_path);
            return 1;
        }

        strcpy(new_path, last_dir); // set new_path to last_dir
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
        if (last_dir) strcpy(last_dir, cwd);

        if (getcwd(cwd, PATH_MAX) == NULL) {
            perror("cd");
            return 1;
        }
    }

    return 0;
}

