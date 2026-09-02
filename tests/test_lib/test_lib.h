#ifndef TEST_LIB_H
#define TEST_LIB_H

#include <stddef.h>


/* Assertions */

void assert_eq_str(const char* str1, const char* str2,
                   const char* func, const char* file, int line);
void assert_eq_long(long num1, long num2,
                    const char* func, const char* file, int line);

void assert_eq_str_arr(char** arr1, char** arr2,
                       const char* func, const char* file, int line);


/* Test Runner */

typedef void (*test_callback_t)(void);

void run_tests_impl(
    const char* name,
    const test_callback_t funcs[],
    size_t count
);


/* Macros */

#define ASSERT_EQ(X, Y) _Generic(X,     \
        char*:       assert_eq_str,     \
        const char*: assert_eq_str,     \
        long:        assert_eq_long,    \
        int:         assert_eq_long,    \
        char**:      assert_eq_str_arr, \
        default:     assert_eq_long     \
)(X, Y, __func__, __FILE__, __LINE__)


#define RUN_TESTS(name, ...)                       \
    do {                                           \
        test_callback_t funcs[] = { __VA_ARGS__ }; \
        run_tests_impl(                            \
            name,                                  \
            funcs,                                 \
            sizeof funcs / sizeof(test_callback_t) \
        );                                         \
    } while (0);

#endif
