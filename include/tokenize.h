#ifndef TOKENIZE_H
#define TOKENIZE_H

void free_args(char**);

size_t tokenize(const char* command, char** args, size_t max_args);

#endif

