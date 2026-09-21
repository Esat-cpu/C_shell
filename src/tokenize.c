#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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


// Inserts a token to token array with given attributes
static int flush_token(char* buf,
                        size_t len,
                        QuoteType status,
                        Token* tokens,
                        size_t iter,
                        TokenType type) {
    // Allow empty entries in quoted modes but not in NORMAL mode
    if (len == 0 && status == NORMAL) return 0;

    buf[len] = '\0';
    tokens[iter].value = strdup(buf);
    tokens[iter].quote_type = status;
    tokens[iter].token_type = type;
    return 1;
}


// tokenize
size_t tokenize(const char* input, Token* tokens, size_t max_tokens) {
    QuoteType status = NORMAL;
    TokenType type = T_WORD;
    bool escape = false;
    bool space = true;

    size_t iter = 0; // for tokens

    char buf[MAX_BUF];
    size_t len = 0; // for buf


    for (const char* ch = input; *ch; ch++) {
        if (len >= MAX_BUF - 1) {
            int f = flush_token(buf, len, status, tokens, iter, type);
            if (f) { iter++; len = 0; }
        }

        if (iter >= max_tokens - 1) break;

        //  If escape status is 1 and the status is NORMAL,
        //+ append the current character as-is and continue.
        //  If the status is DOUBLE_Q,
        //+ escape only special characters in double quotes.
        //+ With other characters take the '\' character as well.
        //  Also if the current character is space,
        //+ the space flag will not be changed.
        if (escape) {
            const char* special_characters = "$\"\\";
            if (status == DOUBLE_Q && !strchr(special_characters, *ch)) {
                buf[len++] = '\\';
            }

            buf[len++] = *ch;
            escape = false;
            continue;
        }

        //  Check if character is '\', escape status is 0 and
        //+ it isn't enclosed in single quotes
        //  if so, do not take the '\' character and set
        //+ escape status to 1
        if (*ch == '\\' && !escape && status != SINGLE_Q) {
            // If the character after escape character is '$',
            // switch escape to special character '\x01' for the
            // parameter expansion.
            if (*(ch+1) == '$') {
                buf[len++] = '\x01';
                continue;
            }

            escape = true;
            continue;
        }

        // Semicolon case
        if (*ch == ';' && status == NORMAL) {
            int f = flush_token(buf, len, NORMAL, tokens, iter, type);
            if (f) { iter++; len = 0; }

            strcpy(buf, ";");
            flush_token(buf, strlen(buf), NORMAL, tokens, iter, T_SEMI);
            iter++;
            continue;
        }

        // Operator cases
        // PIPE
        if (*ch == '|' && status == NORMAL) {
            const char *next_ch = ch+1;

            int f = flush_token(buf, len, NORMAL, tokens, iter, type);
            if (f) { iter++; len = 0; }

            // PIPE Operator (|)
            if (*next_ch != '|') {
                strcpy(buf, "|");
                flush_token(buf, strlen(buf), NORMAL, tokens, iter, T_PIPE);
                iter++;
                continue;
            }
            // OR Operator (||)
            else {
                strcpy(buf, "||");
                flush_token(buf, strlen(buf), NORMAL, tokens, iter, T_OR);
                iter++;
                ch++;
                continue;
            }
        }

        // AND
        if (*ch == '&' && status == NORMAL) {
            const char *next_ch = ch+1;

            // AND Operator (&&)
            if (*next_ch == '&') {
                int f = flush_token(buf, len, NORMAL, tokens, iter, type);
                if (f) { iter++; len = 0; }

                strcpy(buf, "&&");
                flush_token(buf, strlen(buf), NORMAL, tokens, iter, T_AND);
                iter++;
                ch++;
                continue;
            }
        }

        // Redirection out
        if (*ch == '>' && status == NORMAL) {
            const char *next_ch = ch+1;

            int f = flush_token(buf, len, NORMAL, tokens, iter, type);
            if (f) { iter++; len = 0; }

            TokenType redir_type;

            // >> operator
            if (*next_ch == '>') {
                strcpy(buf, ">>");
                redir_type = T_REDIR_OUT_APPEND;
                ch++;
            }
            // > operator
            else {
                strcpy(buf, ">");
                redir_type = T_REDIR_OUT;
            }

            flush_token(buf, strlen(buf), NORMAL, tokens, iter, redir_type);
            iter++;
            continue;
        }

        // Redirection out for fileno 2 (stderr)
        // It requires a space before it
        if (*ch == '2' && space && status == NORMAL) {
            const char *next_ch = ch+1;

            // If it is a redirection operator
            if (*next_ch == '>') {
                int f = flush_token(buf, len, NORMAL, tokens, iter, type);
                if (f) { iter++; len = 0; }

                const char *next_next_ch = NULL;
                if (*next_ch) next_next_ch = next_ch+1;
                TokenType redir_type;

                // 2>> operator
                if (next_next_ch && *next_next_ch == '>') {
                    strcpy(buf, "2>>");
                    redir_type = T_REDIR_ERR_OUT_APPEND;
                    ch += 2;
                }
                // 2> operator
                else {
                    strcpy(buf, "2>");
                    redir_type = T_REDIR_ERR_OUT;
                    ch++;
                }

                flush_token(
                        buf, strlen(buf), NORMAL, tokens, iter, redir_type);
                iter++;
                continue;
            }
        }

        // comment case
        if (*ch == '#' && space) {
            tokens[iter].value = NULL;
            return iter;
        }

        // space case in normal mode
        if (*ch == ' ' && status == NORMAL) {
            int f = flush_token(buf, len, NORMAL, tokens, iter, type);
            if (f) { iter++; len = 0; }

            space = true;
            continue;
        }
        space = false;


        // double quote case
        if (*ch == '"') {
            if (status == NORMAL) {
                status = DOUBLE_Q;
                continue;
            }

            if (status == DOUBLE_Q) {
                int f = flush_token(buf, len, DOUBLE_Q, tokens, iter, type);
                if (f) { iter++; len = 0; }

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
                int f = flush_token(buf, len, SINGLE_Q, tokens, iter, type);
                if (f) { iter++; len = 0; }

                status = NORMAL;
                continue;
            }
        }

        // normal character case
        buf[len++] = *ch;
    }

    if (flush_token(buf, len, NORMAL, tokens, iter, type)) ++iter;
    tokens[iter].value = NULL;
    return iter; // the index of NULL
}
