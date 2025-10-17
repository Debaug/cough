#include <string.h>

#include "tokenizer/tokenizer.h"

#include "tests/common.h"

int main(int argc, char const* argv[]) {
    char const* source = "() : = := -> => ; + hello fn \n false true";
    TestReporter reporter = test_reporter_new();

    ArrayBuf(Token) tokens = array_buf_new(Token)();
    tokenize(
        (String){ .data = source, .len = strlen(source) },
        &reporter.base,
        &tokens
    );

    assert(reporter.error_codes.len == 0);

    assert(tokens.len == 13);
    for (usize i = 0; i < 13; i++) {
        assert(tokens.data[i].kind == i);
    }

    return 0;
}
