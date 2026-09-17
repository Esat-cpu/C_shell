#ifndef PARSER_H
#define PARSER_H

#include "tokenize.h"
#include "ast.h"


typedef struct {
    Token* tokens;
    size_t pos;
    size_t end;
    char* error_message;
} Parser;


Node* parse(Token* tokens, size_t token_count, char **error_out);

Node* parse_sequence(Parser* p);

Node* parse_logical(Parser* p);

Node* parse_pipe(Parser* p);

Node* parse_command(Parser* p);

#endif
