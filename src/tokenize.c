#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "tokenize.h"

#define MAX_BUF 4096


// Frees strings of tokens
void free_tokens(Token* tokens) {
    for (int i = 0; tokens[i].value; ++i)
        free(tokens[i].value);
}


// Fills a allocated array with only strings of tokens
void tokens_to_str_arr(Token* tokens, char** arr) {
    int i;
    for (i = 0; tokens[i].value; ++i)
        arr[i] = tokens[i].value;
    arr[i] = NULL;
}


// Adds a token to token array with given attributes
static void flush_token(char* buf, size_t* len, QuoteType status, Token* tokens, size_t* iter) {
    if (*len == 0) return;
    buf[*len] = '\0';
    tokens[*iter].value = strdup(buf);
    tokens[*iter].quote_type = status;
    (*iter)++;
    *len = 0;
}


// tokenize
size_t tokenize(const char* input, Token* tokens, size_t max_tokens) {
    QuoteType status = NORMAL;
    int escape = 0;

    size_t iter = 0; // for tokens

    char buf[MAX_BUF];
    size_t len = 0; // for buf


    for (const char* ch = input; *ch; ch++) {
        if (len >= MAX_BUF - 1)
            flush_token(buf, &len, status, tokens, &iter);

        if (iter >= max_tokens - 1) break;

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
            flush_token(buf, &len, NORMAL, tokens, &iter);
            continue;
        }


        // double quote case
        if (*ch == '"') {
            if (status == NORMAL) {
                status = DOUBLE_Q;
                continue;
            }

            if (status == DOUBLE_Q) {
                flush_token(buf, &len, DOUBLE_Q, tokens, &iter);
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
                flush_token(buf, &len, SINGLE_Q, tokens, &iter);
                status = NORMAL;
                continue;
            }
        }

        buf[len++] = *ch;
    }

    flush_token(buf, &len, NORMAL, tokens, &iter);
    tokens[iter].value = NULL;
    return iter; // the index of NULL
}
