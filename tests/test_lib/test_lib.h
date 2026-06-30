#ifndef TEST_LIB_H
#define TEST_LIB_H


void assert_eq_str(const char* str1, const char* str2,
                   const char* desc, const char* file, int line);
void assert_eq_long(long num1, long num2,
                    const char* desc, const char* file, int line);

void assert_eq_str_arr(char** arr1, char** arr2,
                       const char* desc, const char* file, int line);

void end(const char* name);


#define ASSERT_EQ(X, Y, desc) _Generic(X, \
        char*: assert_eq_str, \
        const char*: assert_eq_str, \
        long: assert_eq_long, \
        int: assert_eq_long, \
        char**: assert_eq_str_arr, \
        default: assert_eq_long \
)(X, Y, desc, __FILE__, __LINE__)

#define END(name) end(name)

#endif

