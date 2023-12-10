#pragma once

#include "text/text.h"
#include "util/array.h"

typedef enum token_type {
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,

    TOKEN_COLON_COLON,
    TOKEN_COLON_EQUAL,
    TOKEN_EQUAL,

    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_ARROW,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    TOKEN_EQUAL_EQUAL,
    TOKEN_BANG_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_BANG,

    TOKEN_STRING,
    TOKEN_INTEGER,

    TOKEN_IDENTIFIER,

    TOKEN_EOF,
} token_type_t;

typedef struct token {
    token_type_t type;
    text_span_t span;
} token_t;

typedef struct scanner {
    const char* text;
    text_pos_t current_pos;
} scanner_t;

typedef enum scan_error_kind {
    SCAN_UNEXPECTED_CHARACTER,
    SCAN_INCOMPLETE_TOKEN,
} scan_error_kind_t;

typedef struct scan_error {
    scan_error_kind_t kind;
    text_pos_t pos;
} scan_error_t;

typedef struct scan_result {
    bool success: 1;
    union {
        scan_error_t error;
        token_t token;
    };
} scan_result_t;

scanner_t new_scanner(const char* text);
char peek_scanner(scanner_t scanner);
void step_scanner(scanner_t* scanner);

scan_result_t scan_string(scanner_t* scanner);
scan_result_t scan_integer(scanner_t* scanner);
scan_result_t scan_identifier(scanner_t* scanner);
scan_result_t scan_punctuation(scanner_t* scanner);
scan_result_t scan_one(scanner_t* scanner);
array_buf_t /* token_t */ scan(scanner_t* scanner);
