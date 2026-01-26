#ifndef TEST_LIB_H
#define TEST_LIB_H


void assert_eq_str (const char*, const char*, const char*, const char*, int);
void assert_eq_long (long, long, const char*, const char*, int);

void assert_eq_str_arr (char **, char **, const char*, const char*, int);

void end(const char*);


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
