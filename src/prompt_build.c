#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "prompt_build.h"
#include "shell.h"


void prompt_build(char* prompt, size_t prompt_size) {
    // The path that will appear in the prompt
    char prmpt_cwd[PATH_MAX];

    // '~' contraction for prompt
    if (shell.home && strncmp(shell.cwd, shell.home, strlen(shell.home)) == 0)
        snprintf(prmpt_cwd, PATH_MAX, "~%s", shell.cwd + strlen(shell.home));
    else
        strcpy(prmpt_cwd, shell.cwd);

    // Showing the error code in the prompt if it is not 0
    if (shell.exit_code == 0)
        snprintf(prompt, prompt_size,
                "\033[1;32m%s \033[1;34m%s\033[0m> ",
                shell.user, prmpt_cwd);

    else if (shell.exit_code == 130)
        snprintf(prompt, prompt_size,
                "\033[1;32m%s \033[1;34m%s \033[1;31m[%s]\033[0m> ",
                shell.user, prmpt_cwd, "SIGINT");

    else
        snprintf(prompt, prompt_size,
                "\033[1;32m%s \033[1;34m%s \033[1;31m[%d]\033[0m> ",
                shell.user, prmpt_cwd, shell.exit_code);
}
