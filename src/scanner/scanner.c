#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "scanner/scanner.h"

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

bool is_unary_operator(token_t token) {
    switch (token.type) {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_BANG:
        return true;
    default:
        return false;
    }
}

scan_result_t scan_string(scanner_t* scanner) {
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;
    step_scanner(scanner); // '"' character
    while (peek_scanner(*scanner) != '"') {
        if (peek_scanner(*scanner) == '\0') {
            return RESULT_ERROR(scan_result_t, SCAN_INCOMPLETE_TOKEN);
        }
        if (peek_scanner(*scanner) == '\\') {
            step_scanner(scanner);
        }
        if (peek_scanner(*scanner) != '\0') {
            step_scanner(scanner);
        }
    }
    text_view_t view = {
        .ptr = text,
        .len = scanner->text - text,
        .start = start,
        .end = scanner->text_pos
    };
    token_t token = { .type = TOKEN_STRING, .text = view };
    return RESULT_OK(scan_result_t, token);
}

scan_result_t scan_integer(scanner_t* scanner) {
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;

    if (peek_scanner(*scanner) == '-') {
        step_scanner(scanner);
    }

    while (isdigit(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    text_view_t view = {
        .ptr = text,
        .len = scanner->text - text,
        .start = start,
        .end = scanner->text_pos
    };
    token_t token = { .type = TOKEN_INTEGER, .text = view };
    return RESULT_OK(scan_result_t, token);
}

typedef struct keyword {
    const char* text;
    token_type_t token_type;
} keyword_t;

static keyword_t keywords[] = {
    { "mut", TOKEN_MUT },
    { "fn", TOKEN_FN },
    { "and", TOKEN_AND },
    { "or", TOKEN_OR },
    { "let", TOKEN_LET },
    { "if", TOKEN_IF },
    { "elif", TOKEN_ELIF },
    { "else", TOKEN_ELSE },
    { "loop", TOKEN_LOOP, },
    { "while", TOKEN_WHILE },
    { "break", TOKEN_BREAK },
    { "return", TOKEN_RETURN },
};

scan_result_t scan_identifier_or_keyword(scanner_t* scanner) {
    text_pos_t start = scanner->text_pos;
    const char* text = scanner->text;
    step_scanner(scanner); // first character
    while (isalnum(peek_scanner(*scanner)) || peek_scanner(*scanner) == '_') {
        step_scanner(scanner);
    }
    text_view_t view = {
        .ptr = text,
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
        if (strncmp(keyword.text, view.ptr, view.len) == 0) {
            token.type = keyword.token_type;
            break;
        }
    }

    return RESULT_OK(scan_result_t, token);
}

scan_result_t scan_one(scanner_t* scanner) {
    while (isspace(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    if (peek_scanner(*scanner) == '\0') {
        text_view_t view = {
            .ptr = scanner->text,
            .len = 0,
            .start = scanner->text_pos,
            .end = scanner->text_pos,
        };
        token_t token = { .type = TOKEN_EOF, .text = view };
        return RESULT_OK(scan_result_t, token);
    }

    char first_char = peek_scanner(*scanner);
    if (first_char == '"') {
        return scan_string(scanner);
    } else if (isdigit(first_char)) {
        return scan_integer(scanner);
    } else if (isalpha(first_char)) {
        return scan_identifier_or_keyword(scanner);
    } else if (first_char == '-') {
        if (isdigit(scanner->text[1])) {
            return scan_integer(scanner);
        }
        return scan_punctuation(scanner);
    } else {
        return scan_punctuation(scanner);
    }
}

token_array_buf_t scan(scanner_t* scanner) {
    token_array_buf_t tokens = new_array_buf(token_t);
    while (true) {
        scan_result_t result = scan_one(scanner);

        if (result.is_ok) {
            array_buf_push(&tokens, result.ok);
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
            report_error(
                "%s at %zu:%zu",
                message,
                result.error.pos.line + 1,
                result.error.pos.column + 1
            );
            step_scanner(scanner);
        }

        if (result.ok.type == TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}
