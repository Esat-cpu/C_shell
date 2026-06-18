#ifndef EXPANSION_H
#define EXPANSION_H

#include "tokenize.h"

void expand_param(Token* args, int exit_code);
void expand_tilde(Token* args);

#endif

