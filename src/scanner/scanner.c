#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include "scanner/scanner.h"

scanner_t new_scanner(const char* text) {
    return (scanner_t){ .text = text, .current_pos = {0} };
}

char peek_scanner(scanner_t scanner) {
    return scanner.text[scanner.current_pos.index];
}

void step_scanner(scanner_t* scanner) {
    scanner->current_pos = text_pos_next(scanner->current_pos, scanner->text);
}

scan_result_t scan_string(scanner_t* scanner) {
    text_pos_t start = scanner->current_pos;
    step_scanner(scanner); // '"' character
    while (peek_scanner(*scanner) != '"') {
        if (peek_scanner(*scanner) == '\0') {
            return (scan_result_t){
                .success = false,
                .error = SCAN_INCOMPLETE_TOKEN,
            };
        }
        if (peek_scanner(*scanner) == '\\') {
            step_scanner(scanner);
        }
        if (peek_scanner(*scanner) != '\0') {
            step_scanner(scanner);
        }
    }
    text_span_t span = { .start = start, .end = scanner->current_pos };
    token_t token = { .type = TOKEN_STRING, .span = span };
    return (scan_result_t){
        .success = true,
        .token = token,
    };
}

scan_result_t scan_integer(scanner_t* scanner) {
    text_pos_t start = scanner->current_pos;
    while (isdigit(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    text_span_t span = { .start = start, .end = scanner->current_pos };
    token_t token = { .type = TOKEN_INTEGER, .span = span };
    return (scan_result_t){
        .success = true,
        .token = token,
    };
}

scan_result_t scan_identifier(scanner_t* scanner) {
    text_pos_t start = scanner->current_pos;
    step_scanner(scanner); // first character
    while (isalnum(peek_scanner(*scanner)) || peek_scanner(*scanner) == '_') {
        step_scanner(scanner);
    }
    text_span_t span = { .start = start, .end = scanner->current_pos };
    token_t token = { .type = TOKEN_IDENTIFIER, .span = span };
    return (scan_result_t){
        .success = true,
        .token = token,
    };
}

scan_result_t scan_one(scanner_t* scanner) {
    while (isspace(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    text_pos_t start = scanner->current_pos;
    if (peek_scanner(*scanner) == '\0') {
        text_span_t span = { .start = start, .end = start };
        token_t token = { .type = TOKEN_EOF, .span = span };
        return (scan_result_t){ .success = true, .token = token };
    }

    scan_result_t result;

    char first_char = peek_scanner(*scanner);
    if (first_char == '"') {
        result = scan_string(scanner);
    } else if (isdigit(first_char)) {
        result = scan_integer(scanner);
    } else if (isalpha(first_char)) {
        result = scan_identifier(scanner);
    } else {
        result = scan_punctuation(scanner);
    }

    return result;
}

array_buf_t scan(scanner_t* scanner) {
    array_buf_t tokens = new_array_buf();
    while (true) {
        scan_result_t result = scan_one(scanner);
        if (result.success) {
            array_buf_push(&tokens, &result.token, sizeof(token_t));
        } else {
            const char* message;
            switch (result.error.kind) {
            case SCAN_UNEXPECTED_CHARACTER:
                message = "unexpected character";
                break;
            case SCAN_INCOMPLETE_TOKEN:
                message = "incomplete token";
                break;
            }
            fprintf(stderr, "error at %zu:%zu: %s\n",
                result.error.pos.line + 1, result.error.pos.column + 1,
                message);
            step_scanner(scanner);
        }

        if (result.token.type == TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}
