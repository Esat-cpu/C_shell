#ifndef REDIRECT_H
#define REDIRECT_H

/** Types of redirect operators. */
typedef enum {
    REDIR_NONE,
    REDIR_FILE,  /**< >, >>, <, 2>, 2>> */
    REDIR_DUP    /**< 2>&1, 1>&2, >&2, <&1, etc. */
} RedirType;

/** Result of matching a redirect operator token. */
typedef struct {
    RedirType type;
    int src_fd;
    int error;
    char *filename;
    int append;
    int target_fd;
    int consumed;
} RedirMatch;

/** Parse a redirect operator (and its optional filename argument).
 *  tok may be ">", ">>", "2>&1", etc.
 *  next_tok is the following token (the filename for REDIR_FILE). */
RedirMatch match_redirect(char *tok, char *next_tok);

/** Saved fds and usage flags for restore. */
typedef struct {
    int saved[2];
    int used[2];
} RedirState;

/** Redirect fd to file (dup2 + save old fd). */
void redir_apply_file(RedirState *r, int fd, const char *file, int append);

/** Redirect fd to target_fd (dup2 + save old fd). */
void redir_apply_dup(RedirState *r, int fd, int target);

/** Restore all saved fds (dup2 back). */
void redir_restore(RedirState *r);

#endif
