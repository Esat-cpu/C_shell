#include <string.h>

#include "test_lib.h"
#include "tokenize.h"

#define MAX_ARGS 64


typedef struct {
    char* command;
    char* expected_args[MAX_ARGS];
    const char* desc;
} TestCase;


Token args[MAX_ARGS];
char* arr[MAX_ARGS];


static void
test_one_word() {
    TestCase c = {"helo", {"helo", NULL}, __func__};

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_words_with_space() {
    TestCase c = {"hello world test", {"hello", "world", "test", NULL}, __func__};

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_words_with_escape_and_space() {
    TestCase c = {"hello\\ world test", {"hello world", "test", NULL}, __func__};

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_double_quotes() {
    TestCase c = {
        "echo \"hello 'world'\" test",
        {"echo", "hello 'world'", "test", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_double_quotes_with_escape() {
    TestCase c = {
        "echo \"hello \\\" world\" test",
        {"echo", "hello \" world", "test", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_single_quotes_with_double_quotes_and_escape() {
    TestCase c = {
        "echo '\"hello\" \\' world test",
        {"echo", "\"hello\" \\", "world", "test", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_words_with_many_spaces() {
    TestCase c = {
        "   echo     hello      world   ",
        {"echo", "hello", "world", NULL},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);
    tokens_to_str_arr(args, arr);

    ASSERT_EQ (arr, c.expected_args, c.desc);

    free_tokens(args);
}


static void
test_status_of_normal_tokens() {
    TestCase c = {
        "echo hello",
        {0},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);

    ASSERT_EQ (args[0].status, NORMAL, c.desc);

    free_tokens(args);
}


static void
test_status_of_double_quoted_tokens() {
    TestCase c = {
        "echo \"hello world\"",
        {0},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);

    ASSERT_EQ (args[0].status, NORMAL, c.desc);
    ASSERT_EQ (args[1].status, DOUBLE_Q, c.desc);

    free_tokens(args);
}


static void
test_status_of_single_quoted_tokens() {
    TestCase c = {
        "'echo hello' world",
        {0},
        __func__
    };

    tokenize(c.command, args, MAX_ARGS);

    ASSERT_EQ (args[0].status, SINGLE_Q, c.desc);
    ASSERT_EQ (args[1].status, NORMAL, c.desc);

    free_tokens(args);
}


void
run_tests(void) {
    test_one_word();
    test_words_with_space();
    test_words_with_escape_and_space();
    test_double_quotes();
    test_double_quotes_with_escape();
    test_single_quotes_with_double_quotes_and_escape();
    test_words_with_many_spaces();
    test_status_of_normal_tokens();
    test_status_of_double_quoted_tokens();
    test_status_of_single_quoted_tokens();

    END ("tokenize");
}


int
main() {
    run_tests();
    return 0;
}

