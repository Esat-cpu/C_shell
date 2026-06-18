#ifndef REDIRECT_H
#define REDIRECT_H

typedef enum {
    REDIR_NONE,
    REDIR_FILE,
    REDIR_DUP
} RedirType;

typedef struct {
    RedirType type;
    int src_fd;
    int error;
    char *filename;
    int append;
    int target_fd;
    int consumed;
} RedirMatch;

RedirMatch match_redirect(char *tok, char *next_tok);

typedef struct {
    int saved[2];
    int used[2];
} RedirState;

void redir_apply_file(RedirState *r, int fd, const char *file, int append);
void redir_apply_dup(RedirState *r, int fd, int target);
void redir_restore(RedirState *r);

#endif
