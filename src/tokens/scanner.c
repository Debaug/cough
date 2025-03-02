#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "tokens/scanner.h"

scanner_t new_scanner(const char* text, reporter_t* reporter) {
    text_pos_t start = { .line = 0, .column = 0, .index = 0 };
    return (scanner_t){ .text = text, .text_pos = start, .reporter = reporter };
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

text_view_t scanner_text_from(scanner_t scanner, text_pos_t start) {
    size_t len = scanner.text_pos.index - start.index;
    const char* data = scanner.text - len;
    return (text_view_t){
        .data = data,
        .len = len,
        .start = start,
        .end = scanner.text_pos
    };
}

static result_t scan_string(scanner_t* scanner, token_t* dst) {
    text_pos_t start = scanner->text_pos;
    step_scanner(scanner); // '"' character
    while (peek_scanner(*scanner) != '"') {
        if (peek_scanner(*scanner) == '\0') {
            error_t error = {
                .kind = ERROR_UNCLOSED_STRING,
                .source = scanner_text_from(*scanner, start),
                .message = format("missing closing `\"` in string literal")
            };
            report(scanner->reporter, error);
            return ERROR;
        }
        if (peek_scanner(*scanner) == '\\') {
            step_scanner(scanner);
        }
        if (peek_scanner(*scanner) != '\0') {
            step_scanner(scanner);
        }
    }
    *dst = (token_t){ .type = TOKEN_STRING, .text = scanner_text_from(*scanner, start) };
    return SUCCESS;
}

static result_t scan_integer(scanner_t* scanner, token_t* dst) {
    text_pos_t start = scanner->text_pos;

    if (peek_scanner(*scanner) == '-') {
        step_scanner(scanner);
    }

    while (isdigit(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }

    *dst = (token_t){ .type = TOKEN_INTEGER, .text = scanner_text_from(*scanner, start) };
    return SUCCESS;
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
    { "loop", TOKEN_LOOP, },
    { "while", TOKEN_WHILE },
    { "break", TOKEN_BREAK },
    { "return", TOKEN_RETURN },
};

static result_t scan_identifier_or_keyword(scanner_t* scanner, token_t* dst) {
    text_pos_t start = scanner->text_pos;
    step_scanner(scanner); // first character
    while (isalnum(peek_scanner(*scanner)) || peek_scanner(*scanner) == '_') {
        step_scanner(scanner);
    }
    text_view_t text = scanner_text_from(*scanner, start);
    token_t token = { .type = TOKEN_IDENTIFIER, .text = text };

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        keyword_t keyword = keywords[i];
        if (strlen(keyword.text) != text.len) {
            continue;
        }
        if (strncmp(keyword.text, text.data, text.len) == 0) {
            token.type = keyword.token_type;
            break;
        }
    }

    *dst = token;
    return SUCCESS;
}

static result_t scan_one(scanner_t* scanner, token_t* dst) {
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
        return SUCCESS;
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

token_array_buf_t scan(scanner_t* scanner) {
    token_array_buf_t tokens = new_array_buf(token_t);
    while (true) {
        token_t token;
        result_t result = scan_one(scanner, &token);

        if (result != SUCCESS) {
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
