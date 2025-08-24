#include <stdio.h>
#include <string.h>
#include "trim.h"


struct TrimTestCase {
    char input[128];
    char expected[128];
};

// for adding more tests edit here
static void run_trim_test(void) {
    struct TrimTestCase cases[] = {
        {"   exit ", "exit"},
        {"exit", "exit"},
        {"   ", ""},
        {"", ""},
        {"   hello  world  ", "hello  world"},
        {"test\n", "test"}
    };

    size_t total = sizeof(cases) / sizeof(cases[0]);
    size_t success = 0;

    for (unsigned i = 0; i < total; i++) {
        char buffer[128];
        strcpy(buffer, cases[i].input);

        trim(buffer);

        if (strcmp(buffer, cases[i].expected) == 0) {
            success++;
        } else {
            fprintf(stderr, "[!] Trim failed: \"%s\" -> \"%s\"\nExpected: \"%s\"\n",
                    cases[i].input, buffer, cases[i].expected);
        }
    }

    if (success == total)
            printf("[OK] Trim test successful.\n");
}

int main() {
    run_trim_test();
    return 0;
}

