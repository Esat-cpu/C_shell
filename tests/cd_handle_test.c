// Run it only if you have a $HOME directory like /home/user
// and if you have access to root dir (/) and home dir.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "cd_handle.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


struct CdTestCase {
    char** input;
    char* expected_cwd;
    char* expected_last_dir;
    int expected_code;
    char* desc;
};

static void run_cd_handle_test( void ) {
    char cwd[PATH_MAX];
    char last_dir[PATH_MAX];
    char first_dir[PATH_MAX];
    char* home = getenv("HOME");

    if (getcwd(first_dir, sizeof(first_dir)) == NULL) {
        fprintf(stderr, "Test could not start.\n");
        perror("getcwd");
        return;
    }
    strcpy(cwd, first_dir);
    strcpy(last_dir, first_dir);


    char* cmd1[] = {"cd", home, NULL};
    char* cmd2[] = {"cd", "/", NULL};
    char* cmd3[] = {"cd", "asdf", "asdf", NULL};
    char* cmd4[] = {"cd", "home", NULL};
    char* cmd5[] = {"cd", "-", NULL};
    char* cmd6[] = {"cd", NULL};

    struct CdTestCase cases[] = {
        {cmd1, home, first_dir, 0, "an absolute path"},
        {cmd2, "/", home, 0,       "root dir"},
        {cmd3, "/", home, 1,       "too many args"},
        {cmd4, "/home", "/", 0,    "dir name"},
        {cmd5, "/", "/home", 0,    "previous dir with arg -"},
        {cmd6, home, "/", 0,       "no args"}
    };

    size_t total = sizeof(cases) / sizeof(cases[0]);

    for (unsigned i = 0; i < total; i++) {
        int status = cd_handle(cases[i].input, cwd, last_dir);

        if (
            strcmp(cwd, cases[i].expected_cwd) != 0 ||
            strcmp(last_dir, cases[i].expected_last_dir) != 0 ||
            status != cases[i].expected_code
        ) {
            printf("[!] cd failed with %s.\n", cases[i].desc);
            return;
        }
    }
    printf("\n[OK] cd test successful.\n");
}


int main() {
    run_cd_handle_test();
    return 0;
}

