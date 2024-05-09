#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "tokens/scanner.h"

scanner_t new_scanner(const char* text) {
    return (scanner_t){ .text = text, .text_pos = {0} };
}

char peek_scanner(scanner_t scanner) {
    return *scanner.text;
}

void step_scanner(scanner_t* scanner) {
    scanner->text_pos = text_pos_next(scanner->text_pos, scanner->text);
    scanner->text++;
}

static bool is_unary_operator(token_t token) {
    switch (token.type) {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_BANG:
        return true;
    default:
        return false;
    }
}

static scan_result_t scan_string(scanner_t* scanner, token_t* dst) {
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;
    step_scanner(scanner); // '"' character
    while (peek_scanner(*scanner) != '"') {
        if (peek_scanner(*scanner) == '\0') {
            return SCAN_INCOMPLETE_TOKEN;
        }
        if (peek_scanner(*scanner) == '\\') {
            step_scanner(scanner);
        }
        if (peek_scanner(*scanner) != '\0') {
            step_scanner(scanner);
        }
    }
    text_view_t view = {
        .data = text,
        .len = scanner->text - text,
        .start = start,
        .end = scanner->text_pos
    };
    *dst = (token_t){ .type = TOKEN_STRING, .text = view };
    return SCAN_SUCCESS;
}

static scan_result_t scan_integer(scanner_t* scanner, token_t* dst) {
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;

    if (peek_scanner(*scanner) == '-') {
        step_scanner(scanner);
    }

    while (isdigit(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    text_view_t view = {
        .data = text,
        .len = scanner->text - text,
        .start = start,
        .end = scanner->text_pos
    };
    *dst = (token_t){ .type = TOKEN_INTEGER, .text = view };
    return SCAN_SUCCESS;
}

typedef struct keyword {
    const char* text;
    token_type_t token_type;
} keyword_t;

static keyword_t keywords[] = {
    { "mut", TOKEN_MUT },
    { "struct", TOKEN_STRUCT },
    { "variant", TOKEN_VARIANT },
    { "fn", TOKEN_FN },
    { "and", TOKEN_AND },
    { "or", TOKEN_OR },
    { "let", TOKEN_LET },
    { "if", TOKEN_IF },
    { "elif", TOKEN_ELIF },
    { "else", TOKEN_ELSE },
    { "infinite_loop", TOKEN_LOOP, },
    { "while", TOKEN_WHILE },
    { "break", TOKEN_BREAK },
    { "return", TOKEN_RETURN },
};

static scan_result_t scan_identifier_or_keyword(scanner_t* scanner, token_t* dst) {
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;
    step_scanner(scanner); // first character
    while (isalnum(peek_scanner(*scanner)) || peek_scanner(*scanner) == '_') {
        step_scanner(scanner);
    }
    text_view_t view = {
        .data = text,
        .len = scanner->text - text,
        .start = start,
        .end = scanner->text_pos,
    };
    token_t token = { .type = TOKEN_IDENTIFIER, .text = view };

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        keyword_t keyword = keywords[i];
        if (strlen(keyword.text) != view.len) {
            continue;
        }
        if (strncmp(keyword.text, view.data, view.len) == 0) {
            token.type = keyword.token_type;
            break;
        }
    }

    *dst = token;
    return SCAN_SUCCESS;
}

static scan_result_t scan_one(scanner_t* scanner, token_t* dst) {
    while (isspace(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    if (peek_scanner(*scanner) == '\0') {
        text_view_t view = {
            .data = scanner->text,
            .len = 0,
            .start = scanner->text_pos,
            .end = scanner->text_pos,
        };
        *dst = (token_t){ .type = TOKEN_EOF, .text = view };
        return SCAN_SUCCESS;
    }

    char first_char = peek_scanner(*scanner);
    if (first_char == '"') {
        return scan_string(scanner, dst);
    } else if (isdigit(first_char)) {
        return scan_integer(scanner, dst);
    } else if (isalpha(first_char)) {
        return scan_identifier_or_keyword(scanner, dst);
    } else if (first_char == '-') {
        if (isdigit(scanner->text[1])) {
            return scan_integer(scanner, dst);
        }
        return scan_punctuation(scanner, dst);
    } else {
        return scan_punctuation(scanner, dst);
    }
}

static const char* scan_error_messages[] = {
    [SCAN_UNEXPECTED_CHARACTER] = "unexpected character",
    [SCAN_INCOMPLETE_TOKEN] = "incomplete token",
};

token_array_buf_t scan(scanner_t* scanner) {
    token_array_buf_t tokens = new_array_buf(token_t);
    while (true) {
        token_t token;
        scan_result_t result = scan_one(scanner, &token);

        if (result != SCAN_SUCCESS) {
            report_error(
                "%s at %zu:%zu",
                scan_error_messages[result],
                scanner->text_pos.line + 1,
                scanner->text_pos.column + 1
            );
            step_scanner(scanner);
            continue;
        }

        array_buf_push(&tokens, token);
        if (token.type == TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}
