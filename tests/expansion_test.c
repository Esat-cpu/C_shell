#include <stdlib.h>

#include "test_lib.h"
#include "tokenize.h"
#include "expansion.h"
#include "shell.h"

#define MAX_ARGS 64


typedef struct {
    const char* command;
    char* expected_args[MAX_ARGS];
} TestCase;


Token tokens[MAX_ARGS];
char* arr[MAX_ARGS];


static void
set_up() {
    setenv("TEST_ENV_VAR", "test", 1);

    static char* argv[3] = {"shell", "test_arg", NULL};

    shell.name = "shell";
    shell.argc = 2;
    shell.argv = argv;
}


static void
test_param_expansion_with_home_var() {
    char* env_home = getenv("HOME");
    if (!env_home) env_home = "";

    TestCase c = {
        "echo $HOME",
        {"echo", env_home, NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_unquoted() {
    TestCase c = {
        "echo $TEST_ENV_VAR",
        {"echo", "test", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_double_quoted() {
    TestCase c = {
        "echo \"$TEST_ENV_VAR\"",
        {"echo", "test", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_single_quoted() {
    TestCase c = {
        "echo '$TEST_ENV_VAR'",
        {"echo", "$TEST_ENV_VAR", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_with_slash() {
    TestCase c = {
        "echo /hello/$TEST_ENV_VAR/world",
        {"echo", "/hello/test/world", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_exit_code() {
    shell.exit_code = 42;

    TestCase c = {
        "echo $?",
        {"echo", "42", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


/* Take positional arguments from shell.argv */
static void
test_param_expansion_digits() {
    TestCase c = {
        "echo $1 $2 foo$42bar",
        {"echo", "test_arg", "", "foobar", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_dollar_sign_as_literal() {
    TestCase c = {
        "echo $ $- foo$ 42$",
        {"echo", "$", "$-", "foo$", "42$", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_undeclared_var() {
    unsetenv("UNDECLARED_TEST_VAR");

    TestCase c = {
        "echo $UNDECLARED_TEST_VAR",
        {"echo", "", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


static void
test_param_expansion_with_braces() {
    TestCase c = {
        "echo ${TEST_ENV_VAR} ${1} ${} \\${2}",
        {"echo", "test", "test_arg", "", "${2}", NULL},
    };

    char *msg = NULL;
    tokenize(c.command, tokens, MAX_ARGS);
    expand_param(tokens, &msg);

    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ(arr, c.expected_args);

    free_tokens(tokens);
}


int
main() {
    set_up();
    RUN_TESTS(
        "expansion",
        test_param_expansion_with_home_var,
        test_param_expansion_unquoted,
        test_param_expansion_double_quoted,
        test_param_expansion_single_quoted,
        test_param_expansion_with_slash,
        test_param_expansion_exit_code,
        test_param_expansion_digits,
        test_param_expansion_dollar_sign_as_literal,
        test_param_expansion_undeclared_var,
        test_param_expansion_with_braces,
    );
}
