#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenize.h"

#define BUF_SIZE 256



static int
ensure_capacity (char** buffer, size_t* cap, size_t len, size_t n) {
    if (*cap == 0) *cap = 16;

    while (len + n >= *cap) {
        *cap *= 2;

        char* tmp = realloc(*buffer, *cap);

        if (tmp)
            *buffer = tmp;
        else
            return -1;
    }

    return 0;
}




static void
expand_param_in_token (Token* token, int exit_code) {
    // New buffer for expanded token value
    size_t str_size = BUF_SIZE;
    char* str = malloc(str_size);
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
                snprintf(code, 16, "%d", exit_code);

                if (ensure_capacity(&str, &str_size, len, strlen(code)) < 0) {
                    free(str);
                    return;
                }

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

                    if (ensure_capacity(&str, &str_size, len, env_size) < 0) {
                        free(str);
                        return;
                    }

                    for (int j = 0; env[j]; ++j) {
                        str[len++] = env[j];
                    }
                }
            }

            else {
                // treat '$' as literal
                if (ensure_capacity(&str, &str_size, len, 1) < 0) {
                    free(str);
                    return;
                }

                str[len++] = *ch;
                i++;
            }

        }

        else {
            if (ensure_capacity(&str, &str_size, len, 1) < 0) {
                free(str);
                return;
            }

            str[len++] = *ch;
            i++;
        }
    }

    str[len] = '\0';


    free(token->value);
    token->value = str;
}






void
expand_param (Token* args, int exit_code) {
    for_each_token (token, args) {
        if (strchr(token->value, '$') && token->status != SINGLE_Q) {
            expand_param_in_token (token, exit_code);
        }
    }
}

