#include <string.h>

#include "diagnostics/diagnostics.h"

#include "tests/vm/function_call.h"

static const struct {
    const char* name;
    int(*test)(void);
} tests[] = {
    { .name = "vm/function_call", .test = test_function_call }
};

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        eprintf("Usage: testcough <testname>\n");
        return -1;
    }
    const char* test_name = argv[1];
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        if (strcmp(test_name, tests[i].name) == 0) {
            return tests[i].test();
        }
    }
    eprintf("error: No test named '%s'.\n", test_name);
    return -1;
}
