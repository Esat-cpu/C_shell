#include <stddef.h>
#include <string.h>

#define MAX_BUF 4096

typedef enum {
    NORMAL=1,
    SINGLE_Q,
    DOUBLE_Q
} Status;



static void flush_token(char* buf, size_t* len, char** args, size_t* iter) {
    if (*len == 0) return;
    buf[*len] = '\0';
    args[(*iter)++] = strdup(buf);
    *len = 0;
}


size_t tokenize(const char* command, char** args, size_t max_args) {
    Status status = NORMAL;
    int escape = 0;

    size_t iter = 0; // for args

    char buf[MAX_BUF];
    size_t len = 0; // for buf


    for (const char* ch = command; *ch; ch++) {
        if (len >= MAX_BUF - 1)
            flush_token(buf, &len, args, &iter);

        if (iter >= max_args - 1) break;

        if (escape) {
            buf[len++] = *ch;
            escape = 0;
            continue;
        }

        //  Check if character is '\', escape status is 0 and
        //+ it isn't enclosed in single quotes
        //  if so, do not take the '\' character and set
        //+ escape status to 1
        if (*ch == '\\' && !escape && status != SINGLE_Q) {
            escape = 1;
            continue;
        }

        if (*ch == ' ' && status == NORMAL) {
            flush_token(buf, &len, args, &iter);
            continue;
        }


        // double quote case
        if (*ch == '"') {
            if (status == NORMAL) {
                status = DOUBLE_Q;
                continue;
            }

            if (status == DOUBLE_Q) {
                flush_token(buf, &len, args, &iter);
                status = NORMAL;
                continue;
            }
        }

        // single quote case
        if (*ch == '\'') {
            if (status == NORMAL) {
                status = SINGLE_Q;
                continue;
            }

            if (status == SINGLE_Q) {
                flush_token(buf, &len, args, &iter);
                status = NORMAL;
                continue;
            }
        }

        buf[len++] = *ch;
    }

    flush_token(buf, &len, args, &iter);
    args[iter] = NULL;
    return iter; // the index of NULL
}

