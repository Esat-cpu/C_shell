#ifndef TOKENIZE_H
#define TOKENIZE_H

/** Iterate over a NULL-sentinel-terminated Token array. */
#define for_each_token(token, args) \
    for (Token* (token) = (args); (token)->value; ++(token))

/** Quoting context of a token during tokenization. */
typedef enum {
    NORMAL=1,   /**< Not quoted. */
    SINGLE_Q,   /**< Inside single quotes. */
    DOUBLE_Q    /**< Inside double quotes. */
} Status;

/** A single token resulting from splitting a command string. */
typedef struct {
    char* value;
    Status status;
} Token;

/** Convert a Token array to a plain char* array (for exec). */
void tokens_to_str_arr(Token*, char**);

/** Free values inside a Token array. */
void free_tokens(Token*);

/** Split command into tokens, respecting quotes.
 *  Returns the number of tokens written. */
size_t tokenize(const char* command, Token* args, size_t max_args);

#endif

