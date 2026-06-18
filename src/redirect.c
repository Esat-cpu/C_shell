#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "redirect.h"

RedirMatch
match_redirect(char *tok, char *next_tok) {
    RedirMatch m = {REDIR_NONE};

    if (strcmp(tok, ">") == 0) {
        if (!next_tok) { m.error = 1; return m; }
        m.type = REDIR_FILE; m.src_fd = STDOUT_FILENO;
        m.filename = next_tok; m.consumed = 2;
        return m;
    }
    if (strcmp(tok, ">>") == 0) {
        if (!next_tok) { m.error = 1; return m; }
        m.type = REDIR_FILE; m.src_fd = STDOUT_FILENO;
        m.filename = next_tok; m.append = 1; m.consumed = 2;
        return m;
    }
    if (strcmp(tok, "2>") == 0) {
        if (!next_tok) { m.error = 1; return m; }
        m.type = REDIR_FILE; m.src_fd = STDERR_FILENO;
        m.filename = next_tok; m.consumed = 2;
        return m;
    }
    if (strcmp(tok, "2>>") == 0) {
        if (!next_tok) { m.error = 1; return m; }
        m.type = REDIR_FILE; m.src_fd = STDERR_FILENO;
        m.filename = next_tok; m.append = 1; m.consumed = 2;
        return m;
    }

    if (tok[0] == '>' && tok[1] != '\0' && tok[1] != '>' && tok[1] != '&') {
        m.type = REDIR_FILE; m.src_fd = STDOUT_FILENO;
        m.filename = tok + 1; m.consumed = 1;
        return m;
    }
    if (tok[0] == '>' && tok[1] == '>' && tok[2] != '\0') {
        m.type = REDIR_FILE; m.src_fd = STDOUT_FILENO;
        m.filename = tok + 2; m.append = 1; m.consumed = 1;
        return m;
    }
    if (tok[0] == '2' && tok[1] == '>' && tok[2] != '\0' && tok[2] != '>') {
        m.type = REDIR_FILE; m.src_fd = STDERR_FILENO;
        m.filename = tok + 2; m.consumed = 1;
        return m;
    }
    if (tok[0] == '2' && tok[1] == '>' && tok[2] == '>' && tok[3] != '\0') {
        m.type = REDIR_FILE; m.src_fd = STDERR_FILENO;
        m.filename = tok + 3; m.append = 1; m.consumed = 1;
        return m;
    }

    if (tok[0] == '>' && tok[1] == '&' && tok[2] != '\0') {
        m.type = REDIR_DUP; m.src_fd = STDOUT_FILENO;
        m.target_fd = tok[2] - '0'; m.consumed = 1;
        return m;
    }
    if (tok[0] == '2' && tok[1] == '>' && tok[2] == '&' && tok[3] != '\0') {
        m.type = REDIR_DUP; m.src_fd = STDERR_FILENO;
        m.target_fd = tok[3] - '0'; m.consumed = 1;
        return m;
    }
    if (tok[0] == '1' && tok[1] == '>' && tok[2] == '&' && tok[3] != '\0') {
        m.type = REDIR_DUP; m.src_fd = STDOUT_FILENO;
        m.target_fd = tok[3] - '0'; m.consumed = 1;
        return m;
    }

    return m;
}


void
redir_apply_file(RedirState *r, int fd, const char *file, int append) {
    int idx = (fd == STDERR_FILENO);
    if (!r->used[idx]) {
        r->saved[idx] = dup(fd);
        r->used[idx] = 1;
    }
    int f = open(file, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
    dup2(f, fd);
    close(f);
}

void
redir_apply_dup(RedirState *r, int fd, int target) {
    int idx = (fd == STDERR_FILENO);
    if (!r->used[idx]) {
        r->saved[idx] = dup(fd);
        r->used[idx] = 1;
    }
    dup2(target, fd);
}

void
redir_restore(RedirState *r) {
    if (r->used[0]) {
        dup2(r->saved[0], STDOUT_FILENO);
        close(r->saved[0]);
    }
    if (r->used[1]) {
        dup2(r->saved[1], STDERR_FILENO);
        close(r->saved[1]);
    }
}
