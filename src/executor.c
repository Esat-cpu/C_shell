#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#include "executor.h"
#include "util.h"
#include "tokenize.h"
#include "token.h"
#include "ast.h"
#include "parser.h"
#include "expansion.h"
#include "builtins/builtins.h"
#include "shell.h"

#define MAX_LINE_SIZE 4096


// Set open flags and target fd according to redir_type.
static void collect_redir_config(RedirList* r, int *flags) {
    /* open flags */
    if (r->redir_type == T_REDIR_OUT || r->redir_type == T_REDIR_ERR_OUT)
        *flags = O_WRONLY | O_CREAT | O_TRUNC;

    else if (r->redir_type == T_REDIR_IN)
        *flags = O_RDONLY;

    else
        *flags = O_WRONLY | O_CREAT | O_APPEND;

    /* targetfd */
    if (r->redir_type == T_REDIR_OUT || r->redir_type == T_REDIR_OUT_APPEND)
        r->targetfd = STDOUT_FILENO;
    else if (r->redir_type == T_REDIR_IN)
        r->targetfd = STDIN_FILENO;
    else
        r->targetfd = STDERR_FILENO;
}


// Applies redirection operators in one node.
static ExeResult apply_redirections(RedirList* r) {
    int flags;

    while (r) {
        collect_redir_config(r, &flags);
        r->savedfd = dup(r->targetfd);

        int fd = open(r->filename, flags, 0644);
        if (fd == -1) {
            print_err(r->filename, strerror(errno));
            return E_FILE_ERROR;
        }

        dup2(fd, r->targetfd);
        close(fd);

        r = r->next;
    }

    return E_SUCCESS;
}


// Restore fd targets that were redirected by 'apply_redirections' function.
static void restore_redirections(RedirList* r) {
    if (!r) return;

    restore_redirections(r->next);

    dup2(r->savedfd, r->targetfd);
    close(r->savedfd);
}


// This function is meant to be called from a child, then _exit with
// its return value. Call this function only with command (T_WORD) nodes.
// Returns exit code of the builtin command that was executed,
// returns 127 if builtin command or external command is not found,
// returns 126 if an external command's permission is denied,
// and does not return if the external command is executes successfully.
static int execute_cmd_node_for_pipe(Node* node) {
    // Change SIGINT handler to default
    signal(SIGINT, SIG_DFL);

    ExeResult ar = apply_redirections(node->cmd.redir_list);
    if (ar == E_FILE_ERROR)
        return EXIT_FAILURE;

    // Execute with function if the command matches a builtin
    for (size_t i = 0; builtins[i].func; ++i) {
        if (strcmp(node->cmd.argv[0], builtins[i].cmd_name) == 0) {
            int exit_code = builtins[i].func(node->cmd.argc, node->cmd.argv);
            return exit_code;
        }
    }

    // Execute external command.
    execvp(node->cmd.argv[0], node->cmd.argv);

    if (errno == EACCES) {
        print_err(node->cmd.argv[0], strerror(errno));
        return 126;
    }
    else {
        print_err(node->cmd.argv[0], "Command not found...");
        return 127;
    }
}


// Call this function only with command (T_WORD) nodes.
// Stores the executed command's exit code in shell.exit_code.
// Always returns.
static void execute_cmd_node(Node* node) {
    ExeResult ar = apply_redirections(node->cmd.redir_list);
    if (ar == E_FILE_ERROR) {
        shell.exit_code = EXIT_FAILURE;
        return;
    }

    for (size_t i = 0; builtins[i].func; ++i) {
        if (strcmp(node->cmd.argv[0], builtins[i].cmd_name) == 0) {
            shell.exit_code = builtins[i].func(node->cmd.argc, node->cmd.argv);
            restore_redirections(node->cmd.redir_list);
            return;
        }
    }

    int status;
    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        execvp(node->cmd.argv[0], node->cmd.argv);

        if (errno == EACCES) {
            print_err(node->cmd.argv[0], strerror(errno));
            _exit(126);
        }
        else {
            print_err(node->cmd.argv[0], "Command not found...");
            _exit(127);
        }
    }

    else if (pid > 0) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            shell.exit_code = WEXITSTATUS(status);
    }

    else
        print_err("fork", strerror(errno));

    restore_redirections(node->cmd.redir_list);
}


// Call this on first pipe (T_PIPE) node on AST.
static int pipe_traversal(Node* node,
                           size_t *pid_pos, pid_t pids[],
                           int (*pipefd)[2], size_t pipe_count) {
    if (!node || node->type != T_PIPE) return EXIT_SUCCESS;

    Node* arms[2] = {node->operator.left, node->operator.right};

    int result;

    result = pipe_traversal(arms[0], pid_pos, pids, pipefd, pipe_count);
    if (result == EXIT_FAILURE) return EXIT_FAILURE;

    result = pipe_traversal(arms[1], pid_pos, pids, pipefd, pipe_count);
    if (result == EXIT_FAILURE) return EXIT_FAILURE;

    for (size_t i = 0; i < 2; ++i) {
        if (arms[i] && arms[i]->type == T_WORD) {
            pids[*pid_pos] = fork();

            if (pids[*pid_pos] == 0) {
                if (*pid_pos > 0)
                    dup2(pipefd[(*pid_pos)-1][0], STDIN_FILENO);
                if (*pid_pos < pipe_count)
                    dup2(pipefd[*pid_pos][1], STDOUT_FILENO);

                for (size_t p = 0; p < pipe_count; ++p) {
                    close(pipefd[p][0]);
                    close(pipefd[p][1]);
                }

                _exit(execute_cmd_node_for_pipe(arms[i]));
            }

            else if (pids[*pid_pos] > 0)
                (*pid_pos)++;

            else {
                print_err("fork", strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}


// Traverse all pipe nodes and execute them concurrently with pipes
// between them.
static void pipe_handle(Node* node) {
    Node* tmp = node;
    size_t pipe_counter = 0;

    // Count pipes
    while (tmp->type == T_PIPE) {
        pipe_counter++;
        tmp = tmp->operator.left;
    }

    int pipefd[pipe_counter][2];

    for (size_t i = 0; i < pipe_counter; ++i) {
        int p = pipe(pipefd[i]);

        if (p == -1) {
            shell.exit_code = EXIT_FAILURE;
            print_err("pipe", strerror(errno));
            return;
        }
    }

    pid_t pids[pipe_counter + 1];
    size_t pid_pos = 0;
    int status;

    int e = pipe_traversal(node, &pid_pos, pids, pipefd, pipe_counter);

    for (size_t p = 0; p < pipe_counter; ++p) {
        close(pipefd[p][0]);
        close(pipefd[p][1]);
    }

    if (e == EXIT_FAILURE) {
        shell.exit_code = EXIT_FAILURE;

        for (size_t i = 0; i < pid_pos; ++i)
            kill(pids[i], SIGKILL);

        return;
    }

    for (size_t i = 0; i < (pipe_counter + 1); ++i) {
        waitpid(pids[i], &status, 0);
    }

    if (WIFEXITED(status))
        shell.exit_code = WEXITSTATUS(status);
}


// Traverse the AST, apply semantics and execute the commands.
static void execute_ast(Node* root) {
    if (root->type == T_WORD)
        execute_cmd_node(root);

    // Execute left side of semicolon, if there are nodes on right,
    // execute them separately.
    else if (root->type == T_SEMI) {
        execute_ast(root->operator.left);

        if (shell.errexit && shell.exit_code != 0)
            exit(shell.exit_code);

        if (root->operator.right)
            execute_ast(root->operator.right);
    }

    // If the left arm's exit code is 0 (success), execute the right
    // arm too.
    else if (root->type == T_AND) {
        execute_ast(root->operator.left);

        if (shell.exit_code == 0)
            execute_ast(root->operator.right);
    }

    // If the left arm's exit code is non-zero (failure), execute
    // the right arm too.
    else if (root->type == T_OR) {
        execute_ast(root->operator.left);

        if (shell.exit_code != 0)
            execute_ast(root->operator.right);
    }

    // Execute commands below this node with pipe_handle
    else if (root->type == T_PIPE) {
        pipe_handle(root);
    }
}


// Returns E_PARSE_ERROR on parse error and writes its error
// message to error_out.
// On parse error, set exit code to EXIT_FAILURE if it is set
// to 0 (success), otherwise, leave the previous exit code as-is.
ExeResult execute_line(char* line, char **error_out) {
    // Trimming spaces at the start and end of the command.
    trim(line);
    // Return on empty or comment line.
    if (line[0] == '\0' || line[0] == '#')
        return E_SUCCESS;

    TokenArray ta;
    Node* root = NULL;

    tokenize(line, &ta);

    int e = expand_param(&ta, error_out);
    if (e) {
        free_tokens(ta);

        if (shell.exit_code == 0)
            shell.exit_code = EXIT_FAILURE;

        return E_PARSE_ERROR;
    }

    root = parse(ta.tokens, ta.len, error_out);

    if (root == NULL) {
        free_tokens(ta);

        if (shell.exit_code == 0)
            shell.exit_code = EXIT_FAILURE;

        return E_PARSE_ERROR;
    }

    execute_ast(root);

    free_ast(root);
    free_tokens(ta);

    return E_SUCCESS;
}


// Executes a file line by line. Command failures do not stop
// the file execution. Stops at the first parse error; lines
// before the error are already executed.
//
// Returns
// - E_SUCCESS if the file executed successfully.
// - E_PARSE_ERROR if a line contains parse error.
// - E_FILE_ERROR if the file cannot be opened.
ExeResult execute_file(const char* filename) {
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        print_err(filename, strerror(errno));
        shell.exit_code = 127;
        return E_FILE_ERROR;
    }

    char line[MAX_LINE_SIZE];
    size_t line_count = 1;
    char* error_message = NULL;

    while (fgets(line, MAX_LINE_SIZE, file)) {
        ExeResult ex = execute_line(line, &error_message);

        if (ex == E_PARSE_ERROR) {
            fprintf(stderr, "%s: line %zu: %s\n",
                            filename, line_count, error_message);
            fprintf(stderr, "%s: line %zu: '%s'\n",
                            filename, line_count, line);

            free(error_message);
            fclose(file);
            return E_PARSE_ERROR;
        }

        line_count++;
    }

    fclose(file);
    return E_SUCCESS;
}
