#include <stddef.h>

#include "builtins/builtins.h"
#include "builtins/cd.h"
#include "builtins/exit_builtin.h"


struct Builtin builtins[] = {
    { .cmd_name = "cd",     .func = cd           },
    { .cmd_name = "exit",   .func = exit_builtin },
    { NULL, NULL },
};
