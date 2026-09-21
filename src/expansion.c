#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "expansion.h"
#include "tokenize.h"
#include "str_util.h"
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


static void expand_param_in_token(Token* token) {
    char *ch = next_dollar_sign(token->value);
    if (!ch) return;

    // New buffer for expanded token value
    String* str = new_string();
    char *start = token->value;

    do {
        add_span_to_str(str, start, ch);
        char *next = ch+1;

        // Escape case
        if (*ch == '\x01') {
            ch++;
            add_chr_to_str(str, *ch);
            ch++;
        }

        else { /* *ch == '$' */
            // $? case
            // Expand parameter as exit code
            if (*next == '?') {
                add_int_to_str(str, shell.exit_code);
                ch = next+1;
            }

            // $0, $1, $2 ... case
            // Expand parameter as context argument
            else if (isdigit((unsigned char) *next)) {
                int ind = strtol(next, &ch, 10);

                if (ind < shell.argc)
                    add_slice_to_str(str, shell.argv[ind]);
            }

            // $ENV_VARIABLE case
            // Expand parameter as environment variable
            else if (isalnum((unsigned char) *next) || *next == '_') {
                ch = next;
                while ((isalnum((unsigned char) *next) || *next == '_'))
                    next++;

                char* env_name = strndup(ch, (next - ch));

                char* env_value;
                if ((env_value = getenv(env_name)))
                    add_slice_to_str(str, env_value);

                ch = next;
                free(env_name);
            }

            // $$ case
            // Expand parameter as shell's PID
            else if (*next == '$') {
                add_int_to_str(str, (int)getpid());
                ch = next+1;
            }

            // Take dollar sign as-is
            else {
                add_chr_to_str(str, '$');
                ch++;
            }
        }

        start = ch;
    } while ((ch = next_dollar_sign(ch)));

    add_slice_to_str(str, start);

    free(token->value);
    token->value = str->data;
    free(str);
}


void expand_param(Token* tokens) {
    for_each_token (token, tokens) {
        if (strchr(token->value, '$') && token->quote_type != SINGLE_Q) {
            expand_param_in_token(token);
        }
    }
}
