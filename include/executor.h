#ifndef EXECUTOR_H
#define EXECUTOR_H


typedef enum {
    E_SUCCESS=0,
    E_PARSE_ERROR,
    E_FILE_ERROR,
} ExeResult;


ExeResult execute_line(char* line, char **error_out);

ExeResult execute_file(const char* filename);

#endif
