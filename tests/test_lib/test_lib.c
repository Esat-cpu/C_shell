#include <stdio.h>
#include <string.h>

#define GREEN  "\033[0;32m"
#define RED    "\033[0;31m"
#define RESET  "\033[0m"

#define ARR_STR_BUFF_SIZE 4096


static size_t tests = 0;
static size_t test_success = 0;
static size_t test_fail = 0;


// Print the values of a string array to out
static void
sprint_arr(char** args, char* out) {
    if (args == NULL) {
        snprintf(out, ARR_STR_BUFF_SIZE, "NULL");
        return;
    }

    strcpy(out, "{");
    for (int i = 0; args[i]; ++i) {
        strcat(out, "\"");
        strncat(out, args[i], ARR_STR_BUFF_SIZE - strlen(out) - 4);
        strcat(out, "\", ");
    }
    strcat(out, "\b\b}");
}


void
assert_eq_str(const char* str1, const char* str2, const char* desc, const char* file, int line) {
    tests++;

    if (strcmp(str1, str2) == 0) {
        test_success++;
    }
    else {
        test_fail++;

        fprintf(stderr, RED "[!] FAIL at %s:%d: %s\n\"%s\" is not \"%s\"!\n" RESET,
                file, line, desc, str1, str2);
    }
}


void
assert_eq_long(long num1, long num2, const char* desc, const char* file, int line) {
    tests++;

    if (num1 == num2) {
        test_success++;
    }
    else {
        test_fail++;

        fprintf(stderr, RED "[!] FAIL at %s:%d: %s\n%ld is not %ld!\n" RESET,
                file, line, desc, num1, num2);
    }
}


void
assert_eq_str_arr(char** arr1, char** arr2, const char* desc, const char* file, int line) {
    tests++;
    const char* fail_message = RED "[!] FAIL at %s:%d: %s\n%s is not %s!\n" RESET;

    char arr1_str[ARR_STR_BUFF_SIZE];
    char arr2_str[ARR_STR_BUFF_SIZE];
    sprint_arr(arr1, arr1_str);
    sprint_arr(arr2, arr2_str);


    if (arr1 == NULL && arr2 == NULL) {
        test_success++;
        return;
    }

    if (arr1 == NULL || arr2 == NULL) {
        test_fail++;

        fprintf(stderr, fail_message, file, line, desc, arr1_str, arr2_str);
        return;
    }


    size_t i;
    for (i = 0; arr1[i] && arr2[i]; i++) {
        if (strcmp(arr1[i], arr2[i]) != 0) {
            test_fail++;

            fprintf(stderr, fail_message, file, line, desc, arr1_str, arr2_str);
            return;
        }
    }

    // the case where one array is shorter than the other
    if (arr1[i] != NULL || arr2[i] != NULL) {
        fprintf(stderr, fail_message, file, line, desc, arr1_str, arr2_str);
        test_fail++;
        return;
    }

    test_success++;
}


void
end(const char* name) {
    printf("Ran %zu %s test(s).\n", tests, name);
    printf("Success: %zu  |  Fail: %zu\n", test_success, test_fail);
    if (test_fail == 0)
        printf("[" GREEN "OK" RESET "] %s test successful.\n", name);
}

