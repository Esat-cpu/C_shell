#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "tokenize.h"
#include "expansion.h"

#define BUF_INIT 256


static int
ensure_capacity(char** buf, size_t* cap, size_t len, size_t n) {
    if (*cap == 0) *cap = 16;
    while (len + n >= *cap) {
        *cap *= 2;
        char* tmp = realloc(*buf, *cap);
        if (!tmp) return -1;
        *buf = tmp;
    }
    return 0;
}


static int
append_str(char** buf, size_t* cap, size_t* len, const char* src) {
    size_t n = strlen(src);
    if (ensure_capacity(buf, cap, *len, n) < 0) return -1;
    memcpy(*buf + *len, src, n);
    *len += n;
    return 0;
}


static int
append_char(char** buf, size_t* cap, size_t* len, char c) {
    if (ensure_capacity(buf, cap, *len, 1) < 0) return -1;
    (*buf)[(*len)++] = c;
    return 0;
}


static size_t
expand_exit_code(char** buf, size_t* cap, size_t* len, int code) {
    char s[16];
    snprintf(s, sizeof s, "%d", code);
    if (append_str(buf, cap, len, s) < 0) return (size_t)-1;
    return 2;
}


static size_t
expand_env_var(char** buf, size_t* cap, size_t* len,
               const char* value, size_t i)
{
    const char *start = value + i + 1;
    if (!(isalnum((unsigned char)*start) || *start == '_'))
        return 0;

    const char *end = start;
    while (isalnum((unsigned char)*end) || *end == '_')
        end++;

    size_t var_len = end - start;
    char var[var_len + 1];
    memcpy(var, start, var_len);
    var[var_len] = '\0';

    char *env = getenv(var);
    if (env && append_str(buf, cap, len, env) < 0)
        return (size_t)-1;

    return (size_t)(end - value);
}


static void
expand_param_in_token(Token* token, int exit_code) {
    size_t cap = BUF_INIT;
    char *buf = malloc(cap);
    size_t len = 0;

    size_t i = 0;
    while (token->value[i]) {
        if (token->value[i] != '$') {
            if (append_char(&buf, &cap, &len, token->value[i]) < 0) {
                free(buf); return;
            }
            i++;
            continue;
        }

        char next = token->value[i + 1];
        size_t advance;

        if (next == '?') {
            advance = expand_exit_code(&buf, &cap, &len, exit_code);
        }
        else if (isdigit((unsigned char)next)) {
            const char *end = token->value + i + 1;
            while (isdigit((unsigned char)*end)) end++;
            advance = (size_t)(end - token->value);
        }
        else {
            advance = expand_env_var(&buf, &cap, &len, token->value, i);
        }

        if (advance == (size_t)-1) { free(buf); return; }
        if (advance == 0) {
            if (append_char(&buf, &cap, &len, '$') < 0) {
                free(buf); return;
            }
            i++;
        } else {
            i = advance;
        }
    }

    buf[len] = '\0';
    free(token->value);
    token->value = buf;
}


void
expand_param(Token* args, int exit_code) {
    for_each_token(tok, args) {
        if (strchr(tok->value, '$') && tok->status != SINGLE_Q)
            expand_param_in_token(tok, exit_code);
    }
}


void
expand_tilde(Token* args) {
    for_each_token(tok, args) {
        if (tok->status != NORMAL) continue;
        if (tok->value[0] != '~') continue;

        char* home = getenv("HOME");
        if (!home) continue;

        size_t home_len = strlen(home);
        size_t rest_len = strlen(tok->value + 1);
        char* expanded = malloc(home_len + rest_len + 1);
        if (!expanded) continue;

        memcpy(expanded, home, home_len);
        memcpy(expanded + home_len, tok->value + 1, rest_len + 1);

        free(tok->value);
        tok->value = expanded;
    }
}
