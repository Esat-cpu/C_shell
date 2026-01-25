#include <string.h>
#include "test_lib.h"
#include "tokenize.h"


#define MAX_ARGS 64

typedef struct {
    char* command;
    char* expected_args[MAX_ARGS];
    const char* desc;
} TestCase;


static void
test_one_word () {
    TestCase c = {"helo", {"helo", NULL}, __func__};
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}


static void
test_words_with_space () {
    TestCase c = {"hello world test", {"hello", "world", "test", NULL}, __func__};
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}


static void
test_words_with_escape_and_space () {
    TestCase c = {"hello\\ world test", {"hello world", "test", NULL}, __func__};
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}


static void
test_double_quotes () {
    TestCase c = {
        "echo \"hello 'world'\" test",
        {"echo", "hello 'world'", "test", NULL},
        __func__
    };
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}


static void
test_double_quotes_with_escape () {
    TestCase c = {
        "echo \"hello \\\" world\" test",
        {"echo", "hello \" world", "test", NULL},
        __func__
    };
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}


static void
test_single_quotes_with_double_quotes_and_escape () {
    TestCase c = {
        "echo '\"hello\" \\' world test",
        {"echo", "\"hello\" \\", "world", "test", NULL},
        __func__
    };
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}


static void
test_words_with_many_spaces () {
    TestCase c = {
        "   echo     hello      world   ",
        {"echo", "hello", "world", NULL},
        __func__
    };
    char* args[MAX_ARGS];

    tokenize(c.command, args, MAX_ARGS);
    ASSERT_EQ (args, c.expected_args, c.desc);
}



void run_tests ( void ) {
    test_one_word();
    test_words_with_space();
    test_words_with_escape_and_space();
    test_double_quotes();
    test_double_quotes_with_escape();
    test_single_quotes_with_double_quotes_and_escape();
    test_words_with_many_spaces();

    END ("tokenize");
}


int main() {
    run_tests();
    return 0;
}

