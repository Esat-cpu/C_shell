#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "tokenize.h"


// Appends a redirection to the end of the linked list, allocating
// the list's head node if it doesn't exist yet.
void append_to_redir_list(RedirList **list,
                            TokenType redir_type,
                            char *filename) {
    // Create RedirList node
    RedirList* node = malloc(sizeof(RedirList));
    node->redir_type = redir_type;
    node->filename = filename;
    node->savedfd = 0;
    node->targetfd = 0;
    node->next = NULL;

    // Add to tail
    if (*list) {
        RedirList* tail = *list;

        while (tail->next)
            tail = tail->next;

        tail->next = node;
    }
    else {
        *list = node;
    }
}


// Frees the RedirList nodes themselves. Does not free filename --
// it points into the token array, freed separately by free_tokens.
void free_redir_list(RedirList *list) {
    while (list) {
        RedirList* temp = list;
        list = list->next;
        free(temp);
    }
}


// Consumes a redirection operator token and the filename that follows it,
// adding it to the command node's redirection list. Advances *pos past
// the filename. Returns NULL on success, or a heap-allocated error
// message if no filename follows the operator.
char* collect_redirection(Token* tokens, size_t *pos, size_t end, Node* node) {
    if ((*pos + 1 < end) && tokens[(*pos)+1].token_type == T_WORD) {
        append_to_redir_list(
                &node->cmd.redir_list,
                tokens[*pos].token_type,
                tokens[(*pos)+1].value);
        *pos += 2; // for operator and file name
        return NULL;
    }
    else {
        char* error_message = malloc(64);
        snprintf(error_message, 64,
                "Parse error near '%s'", tokens[*pos].value);
        return error_message;
    }
}


// Builds a T_WORD (command) node from tokens[start, end), separating
// plain argument words from redirection operators. If the command
// contains an invalid redirection or no word tokens, writes an error
// message to *error_message and returns NULL.
Node* make_cmd_node(Token* tokens,
                    size_t start,
                    size_t end,
                    char** error_out) {
    size_t count = end - start;

    Node* node = malloc(sizeof(Node));
    node->type = T_WORD;
    node->cmd.argv = malloc(sizeof(char*) * (count + 1));
    node->cmd.redir_list = NULL;

    size_t ind = 0;
    size_t i = start;
    while (i < end) {
        // Command words and redirection operators may be interleaved.
        if (tokens[i].token_type == T_WORD)
            node->cmd.argv[ind++] = tokens[i++].value;

        else {
            char* e = collect_redirection(tokens, &i, end, node);
            if (e) {
                *error_out = e;
                free_ast(node);
                return NULL;
            }
        }
    }

    node->cmd.argv[ind] = NULL;

    if (ind == 0) {
        *error_out = malloc(64);

        if (i > 0)
            snprintf(*error_out, 64, "Parse error near '%s'", tokens[i-1].value);
        else if (tokens[i].value != NULL)
            snprintf(*error_out, 64, "Parse error near '%s'", tokens[i].value);
        else
            snprintf(*error_out, 64, "Parse error near newline");

        free_ast(node);
        return NULL;
    }

    return node;
}


// Builds a bare operator node (PIPE, AND, OR, etc.) with no children yet;
// the caller is expected to attach left/right.
Node* make_operator_node(Token token) {
    Node* node = malloc(sizeof(Node));
    node->type = token.token_type;
    node->operator.left = NULL;
    node->operator.right = NULL;
    return node;
}


// Recursively frees an AST. Does not free the strings pointed to
// by argv/filename -- those belong to the token array and are
// freed separately by free_tokens.
void free_ast(Node* root) {
    if (!root) return;

    if (root->type == T_WORD) {
        free(root->cmd.argv);
        free_redir_list(root->cmd.redir_list);
    }
    else {
        free_ast(root->operator.left);
        free_ast(root->operator.right);
    }

    free(root);
}
