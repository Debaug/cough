#pragma once

#include <stdbool.h>

#include "text/text.h"
#include "alloc/array.h"

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
    TOKEN_RIGHT_ARROW,

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
    TOKEN_AMPERSAND,
    TOKEN_TUBE,
    TOKEN_CARET,

    TOKEN_STRING,
    TOKEN_INTEGER,

    TOKEN_IDENTIFIER,

    TOKEN_MUT,
    TOKEN_FN,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_LOOP,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_RETURN,

    TOKEN_EOF,
} token_type_t;

typedef struct token {
    token_type_t type;
    text_view_t text;
} token_t;

typedef struct scanner {
    const char* text;
    text_pos_t text_pos;
} scanner_t;

typedef array_buf_t(token_t) token_array_buf_t;

typedef enum scan_result {
    SCAN_SUCCESS,
    SCAN_UNEXPECTED_CHARACTER,
    SCAN_INCOMPLETE_TOKEN,
} scan_result_t;

scanner_t new_scanner(const char* text);
char peek_scanner(scanner_t scanner);
void step_scanner(scanner_t* scanner);
scan_result_t scan_punctuation(scanner_t* scanner, token_t* dst);
token_array_buf_t scan(scanner_t* scanner);
