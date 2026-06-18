#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "prompt.h"

void
get_prompt(char* prompt, size_t prompt_size, const char* user,
           const char* cwd, const char* home, int code)
{
    char prmpt_cwd[PATH_MAX];

    if (home && strncmp(cwd, home, strlen(home)) == 0)
        snprintf(prmpt_cwd, PATH_MAX, "~%s", cwd + strlen(home));
    else
        snprintf(prmpt_cwd, PATH_MAX, "%s", cwd);

    if (code == 0)
        snprintf(prompt, prompt_size, "\033[1;32m%s \033[1;34m%s\033[0m> ",
                 user, prmpt_cwd);
    else
        snprintf(prompt, prompt_size, "\033[1;32m%s \033[1;34m%s \033[1;31m[%d]\033[0m> ",
                 user, prmpt_cwd, code);
}
