#pragma once

#include "text/text.h"

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
    TOKEN_DOT,

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
    TOKEN_STRUCT,
    TOKEN_VARIANT,
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
typedef array_buf_t(token_type_t) token_type_array_buf_t;

typedef struct token {
    token_type_t type;
    text_view_t text;
} token_t;
typedef array_buf_t(token_t) token_array_buf_t;
