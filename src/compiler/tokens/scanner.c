#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "compiler/diagnostics.h"
#include "compiler/tokens/scanner.h"

Scanner new_scanner(const char* text, Reporter* reporter) {
    TextPos start = { .line = 0, .column = 0, .index = 0 };
    return (Scanner){ .text = text, .text_pos = start, .reporter = reporter };
}

char peek_scanner(Scanner scanner) {
    return *scanner.text;
}

void step_scanner(Scanner* scanner) {
    scanner->text_pos = text_pos_next(scanner->text_pos, scanner->text);
    scanner->text++;
}

void step_scanner_by_same_line(Scanner* scanner, usize steps) {
    scanner->text_pos.column += steps;
    scanner->text_pos.index += steps;
    scanner->text += steps;
}

static bool is_unary_operator(Token token) {
    switch (token.kind) {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_BANG:
        return true;
    default:
        return false;
    }
}

TextView scanner_text_from(Scanner scanner, TextPos start) {
    usize len = scanner.text_pos.index - start.index;
    const char* data = scanner.text - len;
    return (TextView){
        .data = data,
        .len = len,
        .start = start,
        .end = scanner.text_pos
    };
}

static Result scan_string(Scanner* scanner, Token* dst) {
    TextPos start = scanner->text_pos;
    step_scanner(scanner); // skip opening '"' character
    while (peek_scanner(*scanner) != '"') {
        if (peek_scanner(*scanner) == '\0') {
            report_simple_compiler_error(
                scanner->reporter,
                CE_INVALID_TOKEN,
                format("missing closing `\"` in string literal"),
                scanner_text_from(*scanner, start)
            );
            return ERROR;
        }
        if (peek_scanner(*scanner) == '\\') {
            step_scanner(scanner);
        }
        if (peek_scanner(*scanner) != '\0') {
            step_scanner(scanner);
        }
    }
    step_scanner(scanner); // closing '"' character
    *dst = (Token){ .kind = TOKEN_STRING, .text = scanner_text_from(*scanner, start) };
    return SUCCESS;
}

static Result scan_integer(Scanner* scanner, Token* dst) {
    TextPos start = scanner->text_pos;

    if (peek_scanner(*scanner) == '-') {
        step_scanner(scanner);
    }

    while (isdigit(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }

    *dst = (Token){ .kind = TOKEN_INTEGER, .text = scanner_text_from(*scanner, start) };
    return SUCCESS;
}

typedef struct FixedToken {
    const char* text;
    TokenKind token_kind;
} FixedToken;

static FixedToken keywords[] = {
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

static Result scan_identifier_or_keyword(Scanner* scanner, Token* dst) {
    TextPos start = scanner->text_pos;
    step_scanner(scanner); // first character
    while (isalnum(peek_scanner(*scanner)) || peek_scanner(*scanner) == '_') {
        step_scanner(scanner);
    }
    TextView text = scanner_text_from(*scanner, start);
    Token token = { .kind = TOKEN_IDENTIFIER, .text = text };

    for (usize i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        FixedToken keyword = keywords[i];
        if (strlen(keyword.text) != text.len) {
            continue;
        }
        if (strncmp(keyword.text, text.data, text.len) == 0) {
            token.kind = keyword.token_kind;
            break;
        }
    }

    *dst = token;
    return SUCCESS;
}

// important:
// - because of the implementation of `scan_punctuation`, a token which is a prefix of
// another must appear after it in the following array.
// - punctuation must stay on one line.
static FixedToken punctuation[] = {
    { "(", TOKEN_LEFT_PAREN },
    { ")", TOKEN_RIGHT_PAREN },
    { "[", TOKEN_LEFT_BRACKET },
    { "]", TOKEN_RIGHT_BRACKET },
    { "{", TOKEN_LEFT_BRACE },
    { "}", TOKEN_RIGHT_BRACE },

    { "==", TOKEN_EQUAL_EQUAL },
    { "!=", TOKEN_BANG_EQUAL },
    { "<=", TOKEN_LESS_EQUAL },
    { "<", TOKEN_LESS },
    { ">=", TOKEN_GREATER_EQUAL },
    { ">", TOKEN_GREATER },

    { "!", TOKEN_BANG },
    { "&", TOKEN_AMPERSAND },
    { "|", TOKEN_TUBE },
    { "^", TOKEN_CARET },

    { "::", TOKEN_COLON_COLON },
    { ":=", TOKEN_COLON_EQUAL },
    { "=", TOKEN_EQUAL },

    { ":", TOKEN_COLON },
    { ";", TOKEN_SEMICOLON },
    { ",", TOKEN_COMMA },
    { "->", TOKEN_RIGHT_ARROW },
    { ".", TOKEN_DOT },

    { "+", TOKEN_PLUS },
    { "-", TOKEN_MINUS },
    { "*", TOKEN_STAR },
    { "/", TOKEN_SLASH },
    { "%", TOKEN_PERCENT },
};

static Result scan_punctuation(Scanner* scanner, Token* dst) {
    for (usize i = 0; i < sizeof(punctuation) / sizeof(punctuation[0]); i++) {
        FixedToken punct = punctuation[i];
        usize len = strlen(punct.text);
        if (strncmp(scanner->text, punct.text, len) != 0) {
            continue;
        }
        TextView text = {
            .data = scanner->text,
            .start = scanner->text_pos,
            .len = len,
        };
        step_scanner_by_same_line(scanner, len);
        text.end = scanner->text_pos;
        *dst = (Token){ .kind = punct.token_kind, .text = text };
        return SUCCESS;
    }

    TextView error_source = {
        .data = scanner->text,
        .start = scanner->text_pos,
        .end = text_pos_next(scanner->text_pos, scanner->text),
        .len = 1,
    };
    report_simple_compiler_error(
        scanner->reporter,
        CE_INVALID_TOKEN,
        format("unexpected character `%c`", peek_scanner(*scanner)),
        error_source
    );
    return ERROR;
}

static Result scan_one(Scanner* scanner, Token* dst) {
    while (isspace(peek_scanner(*scanner))) {
        step_scanner(scanner);
    }
    if (peek_scanner(*scanner) == '\0') {
        TextView view = {
            .data = scanner->text,
            .len = 0,
            .start = scanner->text_pos,
            .end = scanner->text_pos,
        };
        *dst = (Token){ .kind = TOKEN_EOF, .text = view };
        return SUCCESS;
    }

    char first_char = peek_scanner(*scanner);
    if (first_char == '"') {
        return scan_string(scanner, dst);
    } else if (isdigit(first_char)) {
        return scan_integer(scanner, dst);
    } else if (isalpha(first_char)) {
        return scan_identifier_or_keyword(scanner, dst);
    } else if (first_char == '-' && isdigit(scanner->text[1])) {
        return scan_integer(scanner, dst);
    } else {
        return scan_punctuation(scanner, dst);
    }
}

TokenArrayBuf scan(Scanner* scanner) {
    TokenArrayBuf tokens = new_array_buf(Token);
    while (true) {
        Token token;
        Result result = scan_one(scanner, &token);

        if (result != SUCCESS) {
            step_scanner(scanner);
            continue;
        }

        array_buf_push(&tokens, token);
        if (token.kind == TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}
