#ifndef EXPANSION_H
#define EXPANSION_H

#include "tokenize.h"

/** Expand $? (exit code) and $VAR (environment variables) in-place. */
void expand_param(Token* args, int exit_code);

/** Expand leading ~ and ~user in-place. */
void expand_tilde(Token* args);

#endif

