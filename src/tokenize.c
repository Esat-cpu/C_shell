#include <string.h>
#include <stdlib.h>
#include "tokenize.h"

#define MAX_BUF 4096


void
free_tokens(Token* args) {
    for (int i = 0; args[i].value; ++i)
        free(args[i].value);
}


void
tokens_to_str_arr(Token* args, char** arr) {
    int i;
    for (i = 0; args[i].value; ++i)
        arr[i] = args[i].value;
    arr[i] = NULL;
}


static void
flush_token(char* buf, size_t* len, Status status, Token* args, size_t* iter) {
    if (*len == 0) return;
    buf[*len] = '\0';
    args[*iter].value = strdup(buf);
    args[*iter].status = status;
    (*iter)++;
    *len = 0;
}


size_t
tokenize(const char* command, Token* args, size_t max_args) {
    Status status = NORMAL;
    int escape = 0;

    size_t iter = 0;
    char buf[MAX_BUF];
    size_t len = 0;

    for (const char* ch = command; *ch; ch++) {
        if (len >= MAX_BUF - 1)
            flush_token(buf, &len, status, args, &iter);

        if (iter >= max_args - 1) break;

        if (escape) {
            buf[len++] = *ch;
            escape = 0;
            continue;
        }

        if (*ch == '\\' && status != SINGLE_Q) {
            escape = 1;
            continue;
        }

        if (*ch == ' ' && status == NORMAL) {
            flush_token(buf, &len, NORMAL, args, &iter);
            continue;
        }

        if (*ch == '"') {
            if (status == NORMAL) {
                status = DOUBLE_Q;
                continue;
            }
            if (status == DOUBLE_Q) {
                flush_token(buf, &len, DOUBLE_Q, args, &iter);
                status = NORMAL;
                continue;
            }
        }

        if (*ch == '\'') {
            if (status == NORMAL) {
                status = SINGLE_Q;
                continue;
            }
            if (status == SINGLE_Q) {
                flush_token(buf, &len, SINGLE_Q, args, &iter);
                status = NORMAL;
                continue;
            }
        }

        buf[len++] = *ch;
    }

    flush_token(buf, &len, NORMAL, args, &iter);
    args[iter].value = NULL;
    return iter;
}
