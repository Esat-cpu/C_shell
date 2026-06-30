#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "prompt_build.h"
#include "shell.h"


void prompt_build(char* prompt, size_t buf_size, const char* home, const char* user) {
    // The path that will appear in the prompt
    char prmpt_cwd[PATH_MAX];

    // '~' contraction for prompt
    if (home && strncmp(shell.cwd, home, strlen(home)) == 0)
        snprintf(prmpt_cwd, PATH_MAX, "~%s", shell.cwd + strlen(home));
    else
        strcpy(prmpt_cwd, shell.cwd);

    // Showing the error code in the prompt if it is not 0
    if (shell.exit_code == 0)
        snprintf(prompt, buf_size, "\033[1;32m%s \033[1;34m%s\033[0m> ",
                 user, prmpt_cwd);
    else
        snprintf(prompt, buf_size, "\033[1;32m%s \033[1;34m%s \033[1;31m[%d]\033[0m> ",
                 user, prmpt_cwd, shell.exit_code);
}

