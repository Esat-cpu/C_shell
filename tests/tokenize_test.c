#include "test_lib.h"
#include "tokenize.h"

#define MAX_ARGS 64


typedef struct {
    char* command;
    char* expected_tokens[MAX_ARGS];
    const char* desc;
} TestCase;


Token tokens[MAX_ARGS];
char* arr[MAX_ARGS];


static void
test_one_word() {
    TestCase c = {"helo", {"helo", NULL}, __func__};

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}


//----- Space character tests -----
static void
test_words_with_space() {
    TestCase c = {"hello world test", {"hello", "world", "test", NULL}, __func__};

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}


static void
test_words_with_escape_and_space() {
    TestCase c = {"hello\\ world test", {"hello world", "test", NULL}, __func__};

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}


static void
test_words_with_many_spaces() {
    TestCase c = {
        "   echo     hello      world   ",
        {"echo", "hello", "world", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}


//----- Quote tests -----
static void
test_double_quotes() {
    TestCase c = {
        "echo \"hello 'world'\" test",
        {"echo", "hello 'world'", "test", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}


static void
test_double_quotes_with_escape() {
    TestCase c = {
        "echo \"hello \\\" \\world\" test",
        {"echo", "hello \" \\world", "test", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}


static void
test_single_quotes_with_double_quotes_and_escape() {
    TestCase c = {
        "echo '\"hello\" \\' world test",
        {"echo", "\"hello\" \\", "world", "test", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);

    free_tokens(tokens);
}



static void
test_status_of_normal_tokens() {
    TestCase c = {
        "echo hello",
        {0},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);

    ASSERT_EQ (tokens[0].quote_type, NORMAL, c.desc);

    free_tokens(tokens);
}


static void
test_status_of_double_quoted_tokens() {
    TestCase c = {
        "echo \"hello world\"",
        {0},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);

    ASSERT_EQ (tokens[0].quote_type, NORMAL, c.desc);
    ASSERT_EQ (tokens[1].quote_type, DOUBLE_Q, c.desc);

    free_tokens(tokens);
}


static void
test_status_of_single_quoted_tokens() {
    TestCase c = {
        "'echo hello' world",
        {0},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);

    ASSERT_EQ (tokens[0].quote_type, SINGLE_Q, c.desc);
    ASSERT_EQ (tokens[1].quote_type, NORMAL, c.desc);

    free_tokens(tokens);
}


//----- Operator recognition tests -----
static void
test_token_with_AND_operator() {
    TestCase c = {
        "echo helo && echo world",
        {"echo", "helo", "&&", "echo", "world", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);
    ASSERT_EQ (tokens[2].token_type, T_AND, c.desc);

    free_tokens(tokens);
}


static void
test_token_with_OR_operator() {
    TestCase c = {
        "echo helo || echo world",
        {"echo", "helo", "||", "echo", "world", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);
    ASSERT_EQ (tokens[1].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[2].token_type, T_OR, c.desc);
    ASSERT_EQ (tokens[3].token_type, T_WORD, c.desc);

    free_tokens(tokens);
}


static void
test_token_with_pipe_operator() {
    TestCase c = {
        "echo|cat | tr",
        {"echo", "|", "cat", "|", "tr", NULL},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);
    ASSERT_EQ (tokens[0].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[1].token_type, T_PIPE, c.desc);
    ASSERT_EQ (tokens[2].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[3].token_type, T_PIPE, c.desc);
    ASSERT_EQ (tokens[4].token_type, T_WORD, c.desc);

    free_tokens(tokens);
}


static void
test_token_with_redirection_operators() {
    TestCase c = {
        "echo > first>second 2> and2>>so 2>>on",
        {"echo", ">", "first", ">", "second", "2>", "and2", ">>", "so", "2>>", "on"},
        __func__
    };

    tokenize(c.command, tokens, MAX_ARGS);
    tokens_to_str_arr(tokens, arr);

    ASSERT_EQ (arr, c.expected_tokens, c.desc);
    ASSERT_EQ (tokens[0].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[1].token_type, T_REDIR_OUT, c.desc);
    ASSERT_EQ (tokens[2].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[3].token_type, T_REDIR_OUT, c.desc);
    ASSERT_EQ (tokens[4].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[5].token_type, T_REDIR_ERR_OUT, c.desc);
    ASSERT_EQ (tokens[6].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[7].token_type, T_REDIR_OUT_APPEND, c.desc);
    ASSERT_EQ (tokens[8].token_type, T_WORD, c.desc);
    ASSERT_EQ (tokens[9].token_type, T_REDIR_ERR_OUT_APPEND, c.desc);
    ASSERT_EQ (tokens[10].token_type, T_WORD, c.desc);

    free_tokens(tokens);
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
    test_token_with_AND_operator();
    test_token_with_OR_operator();
    test_token_with_pipe_operator();
    test_token_with_redirection_operators();

    END ("tokenize");
}


int
main() {
    run_tests();
    return 0;
}

