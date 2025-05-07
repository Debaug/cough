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

typedef struct Keyword {
    const char* text;
    TokenKind token_type;
} Keyword;

static Keyword keywords[] = {
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
        Keyword keyword = keywords[i];
        if (strlen(keyword.text) != text.len) {
            continue;
        }
        if (strncmp(keyword.text, text.data, text.len) == 0) {
            token.kind = keyword.token_type;
            break;
        }
    }

    *dst = token;
    return SUCCESS;
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
    } else if (first_char == '-') {
        if (isdigit(scanner->text[1])) {
            return scan_integer(scanner, dst);
        }
        return scan_punctuation(scanner, dst);
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
