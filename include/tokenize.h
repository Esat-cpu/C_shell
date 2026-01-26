#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef enum {
    NORMAL=1,
    SINGLE_Q,
    DOUBLE_Q
} Status;


typedef struct {
    char* value;
    Status status;
} Token;


void tokens_to_str_arr(Token*, char**);

void free_tokens(Token*);

size_t tokenize(const char* command, Token* args, size_t max_args);

#endif

