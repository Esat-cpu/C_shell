#ifndef PROMPT_H
#define PROMPT_H

/** Build a coloured prompt string: "[user\@host cwd] exit_code$".
 *  cwd is shortened relative to home. */
void get_prompt(char* prompt, size_t prompt_size, const char* user,
                const char* cwd, const char* home, int exit_code);

#endif
