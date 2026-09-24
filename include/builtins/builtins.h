#ifndef BUILTINS_H
#define BUILTINS_H

struct Builtin {
    const char* cmd_name;
    int (*func)(int argc, char** argv);
};

extern struct Builtin builtins[];

#endif
