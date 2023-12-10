#include <stdio.h>
#include <stdbool.h>

#include "text/text.h"
#include "scanner/scanner.h"

int main(int argc, const char* argv[]) {
    if (argc >= 3) {
        fprintf(stderr, "error: too many arguments\n");
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "    cough <FILE>\t-- scan file\n");
        fprintf(stderr, "    cough\t\t-- scan from standard input\n");
        return -1;
    }

    FILE* file;
    bool from_file;
    if (argc == 2) {
        file = fopen(argv[1], "r");
        from_file = true;
    } else {
        file = stdin;
        from_file = false;
    };

    read_file_result_t text_result = read_file(file);
    if (text_result.error) {
        fprintf(stderr, "error: failed to read file (error code %d)\n",
            text_result.error);
        if (from_file) {
            fclose(file);
        }
        return -2;
    }
    array_buf_t text_buf = text_result.text;
    const char* text = (const char*)text_buf.ptr;

    scanner_t scanner = new_scanner(text);
    array_buf_t tokens = scan(&scanner);
    for (token_t* token = tokens.ptr; token->type != TOKEN_EOF; token++) {
        int token_len = token->span.end.index - token->span.start.index;
        printf("%zu:%zu .. %zu:%zu: [%d] '%.*s'\n",
            token->span.start.line + 1,
            token->span.start.column + 1,
            token->span.end.line + 1,
            token->span.end.column + 1,
            token->type,
            token_len,
            text + token->span.start.index
        );
    }

    destroy_array_buf(text_buf);
    if (from_file) {
        fclose(file);
    }
    return 0;
}
