#include <stdlib.h>
#include "test_lib.h"
#include "tokenize.h"
#include "expansion.h"


#define MAX_ARGS 64


typedef struct {
    const char* command;
    char* expected_args[MAX_ARGS];
    const char* desc;
} TestCase;


Token args[MAX_ARGS];
char* arr[MAX_ARGS];



static void
set_up () {
    setenv("TEST_ENV_VAR", "test", 1);
}


static void
test_param_expansion_with_home_var () {
    char* env_home = getenv("HOME");
    if (!env_home) env_home = "";

    TestCase c = {
        "echo $HOME",
        {"echo", env_home, NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_unquoted () {
    TestCase c = {
        "echo $TEST_ENV_VAR",
        {"echo", "test", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_double_quoted () {
    TestCase c = {
        "echo \"$TEST_ENV_VAR\"",
        {"echo", "test", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_single_quoted () {
    TestCase c = {
        "echo '$TEST_ENV_VAR'",
        {"echo", "$TEST_ENV_VAR", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_with_slash () {
    TestCase c = {
        "echo /hello/$TEST_ENV_VAR/world",
        {"echo", "/hello/test/world", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_exit_code () {
    TestCase c = {
        "echo $?",
        {"echo", "42", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 42);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_digits () {
    TestCase c = {
        "echo $1 foo$42bar",
        {"echo", "", "foobar", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_dollar_sign_as_literal () {
    TestCase c = {
        "echo $ $$ $- foo$ 42$",
        {"echo", "$", "$$", "$-", "foo$", "42$", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}



static void
test_param_expansion_undeclared_var () {
    unsetenv("UNDECLARED_TEST_VAR");

    TestCase c = {
        "echo $UNDECLARED_TEST_VAR",
        {"echo", "", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    expand_param(args, 0);

    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}





void run_tests ( void ) {
    set_up();
    test_param_expansion_with_home_var();
    test_param_expansion_unquoted();
    test_param_expansion_double_quoted();
    test_param_expansion_single_quoted();
    test_param_expansion_with_slash();
    test_param_expansion_exit_code();
    test_param_expansion_digits();
    test_param_expansion_dollar_sign_as_literal();
    test_param_expansion_undeclared_var();

    END ("expansion");
}



int main() {
    run_tests();
    return 0;
}

