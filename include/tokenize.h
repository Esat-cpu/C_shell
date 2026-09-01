#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <stddef.h>

#define for_each_token(t, tokens) \
    for (Token* (t) = (tokens); (t)->value; ++(t))


typedef enum {
    NORMAL=1,
    SINGLE_Q,
    DOUBLE_Q,
} QuoteType;


typedef enum {
    T_WORD=1,
    T_PIPE,
    T_AND,
    T_OR,
    T_REDIR_OUT,
    T_REDIR_OUT_APPEND,
    T_REDIR_ERR_OUT,
    T_REDIR_ERR_OUT_APPEND,
} TokenType;


typedef struct {
    char* value;
    QuoteType quote_type;
    TokenType token_type;
} Token;


void tokens_to_str_arr(Token* tokens, char** arr);

void free_tokens(Token* tokens);

size_t tokenize(const char* command, Token* tokens, size_t max_tokens);

#endif
