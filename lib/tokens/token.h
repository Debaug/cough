#include "primitives/primitives.h"
#include "collections/array.h"

typedef enum TokenKind {
    TOKEN_PAREN_LEFT,
    TOKEN_PAREN_RIGHT,
    TOKEN_COLON,
    TOKEN_EQUAL,
    TOKEN_COLON_EQUAL,
    TOKEN_ARROW,
    TOKEN_ARROW_DOUBLE,
    TOKEN_SEMICOLON,

    TOKEN_PLUS,

    TOKEN_IDENTIFIER,

    TOKEN_FN,
    TOKEN_FALSE,
    TOKEN_TRUE,
} TokenKind;

typedef struct Token {
    TokenKind kind;
    usize pos;
} Token;

DECL_ARRAY_BUF(Token)
