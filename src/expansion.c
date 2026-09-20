#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "expansion.h"
#include "tokenize.h"
#include "util.h"
#include "shell.h"

#define BUF_SIZE 256


static void ensure_capacity(char** buffer, size_t* cap, size_t len, size_t n) {
    if (*cap == 0) *cap = 16;

    while (len + n >= *cap) {
        *cap *= 2;
        *buffer = srealloc(*buffer, *cap);
    }
}


static void expand_param_in_token(Token* token) {
    // New buffer for expanded token value
    size_t str_size = BUF_SIZE;
    char* str = smalloc(str_size);
    size_t len = 0;


    size_t i = 0;
    while (token->value[i]) {
        char *ch = &token->value[i];


        if (*ch == '$') {
            char *start = (ch + 1); // char after '$'
            char *end = start;

            if (*start == '?') {
                i += 2;
                char code[16];

                // convert exit code to string
                snprintf(code, 16, "%d", shell.exit_code);

                ensure_capacity(&str, &str_size, len, strlen(code));

                // append exit_code to the result string
                for (size_t j = 0; code[j]; ++j) {
                    str[len++] = code[j];
                }
            }


            else if (isdigit((unsigned char) *start)) {
                while (isdigit((unsigned char) *end)) {
                    end++;
                }

                i = (int)(end - token->value);
            }


            else if (isalnum((unsigned char) *start) || *start == '_') {
                while (isalnum((unsigned char) *end) || *end == '_') {
                    end++;
                }

                size_t var_len = end - start;
                i = (int)(end - token->value);

                char var[var_len + 1];
                memcpy(var, start, var_len);
                var[var_len] = '\0';

                char* env = getenv(var);

                if (env) {
                    size_t env_size = strlen(env);

                    ensure_capacity(&str, &str_size, len, env_size);

                    for (int j = 0; env[j]; ++j) {
                        str[len++] = env[j];
                    }
                }
            }

            else {
                // treat '$' as literal
                ensure_capacity(&str, &str_size, len, 1);

                str[len++] = *ch;
                i++;
            }

        }

        else {
            ensure_capacity(&str, &str_size, len, 1);

            str[len++] = *ch;
            i++;
        }
    }

    str[len] = '\0';


    free(token->value);
    token->value = str;
}


void expand_param(Token* tokens) {
    for_each_token (token, tokens) {
        if (strchr(token->value, '$') && token->quote_type != SINGLE_Q) {
            expand_param_in_token(token);
        }
    }
}
