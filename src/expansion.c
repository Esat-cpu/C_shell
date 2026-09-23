#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "expansion.h"
#include "tokenize.h"
#include "str_util.h"
#include "util.h"
#include "shell.h"


// Finds dollar signs and the special escape character that is
// set in tokenizer.
static char* next_dollar_sign(char* buf) {
    while (*buf != '\0') {
        if (*buf == '\x01' || *buf == '$')
            return buf;

        buf++;
    }

    return NULL;
}


static char* expand_param_in_one_expr(char* start, String* str) {
    char* ch   = start;

    // $? case
    // Expand parameter as exit code
    if (*ch == '?') {
        add_int_to_str(str, shell.exit_code);
        return ch+1;
    }

    // $0, $1, $2 ... case
    // Expand parameter as context argument
    else if (isdigit((unsigned char) *ch)) {
        int ind = strtol(start, &ch, 10);

        if (ind < shell.argc)
            add_slice_to_str(str, shell.argv[ind]);
    }

    // $ENV_VARIABLE case
    // Expand parameter as environment variable
    else if (isalnum((unsigned char) *ch) || *ch == '_') {
        while ((isalnum((unsigned char) *ch) || *ch == '_'))
            ch++;

        char* env_name = strndup(start, (ch - start));

        char* env_value;
        if ((env_value = getenv(env_name)))
            add_slice_to_str(str, env_value);

        free(env_name);
    }

    // $$ case
    // Expand parameter as shell's PID
    else if (*ch == '$') {
        add_int_to_str(str, (int)getpid());
        ch++;
    }

    return ch;
}


static int expand_param_in_token(Token* token, char **error_out) {
    // New buffer for expanded token value
    String* str = new_string();

    char *ch    = token->value;
    char *start = token->value;

    while ((ch = next_dollar_sign(ch))) {
        add_span_to_str(str, start, ch);

        // Escape case
        if (*ch == '\x01') {
            ch++;
            add_chr_to_str(str, *ch);
            ch++;
        }

        else { /* *ch == '$' */
            ch++;
            char* pos;

            // ${PARAM} case
            if (*ch == '{') {
                ch++;
                pos = expand_param_in_one_expr(ch, str);

                // Abort expansion on unclosed '{' so the whole line
                // is skipped.
                if (*pos != '}') {
                    free(str->data);
                    free(str);

                    *error_out = smalloc(64);
                    snprintf(*error_out, 64, "Parse error, expected '}'");
                    return -1;
                }

                ch = pos + 1;
            }

            // $PARAM case
            else {
                pos = expand_param_in_one_expr(ch, str);

                // If no case matches, take the dollar sign as-is
                if (pos == ch)
                    add_chr_to_str(str, '$');
                else
                    ch = pos;
            }
        }

        start = ch;
    }

    add_slice_to_str(str, start);

    free(token->value);
    token->value = str->data;
    free(str);
    return 0;
}


int expand_param(Token* tokens, char **error_out) {
    for_each_token (token, tokens) {
        if (strchr(token->value, '$') && token->quote_type != SINGLE_Q) {
            int e = expand_param_in_token(token, error_out);
            if (e) return e;
        }
    }
    return 0;
}
