#include "primitives/primitives.h"
#include "collections/array.h"
#include "collections/hash_map.h"

typedef enum TokenKind {
    TOKEN_PAREN_LEFT,
    TOKEN_PAREN_RIGHT,
    TOKEN_COLON,
    TOKEN_EQUAL,
    TOKEN_COLON_EQUAL,
    TOKEN_ARROW,
    TOKEN_ARROW_DOUBLE,
    TOKEN_SEMICOLON,

    TOKEN_AMPERSAND,

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

typedef struct TokenStream {
    ArrayBuf(Token) tokens;
    HashMap(usize, usize) _end_pos;
} TokenStream;

Range token_range(TokenStream stream, Token token);
void token_stream_free(TokenStream* token_stream);
