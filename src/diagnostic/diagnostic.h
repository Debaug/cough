#pragma once

#include <stdio.h>

#include "text/text.h"
#include "alloc/array.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define KEY(n) "\x1B[" n "m"
#define KEY_RESET KEY("0")
#define KEY_BOLD KEY("1")
#define KEY_RED KEY("31")
#define KEY_BRIGHT_RED KEY("91")
#define KEY_BRIGHT_CYAN KEY("96")
#define KEY_UNDERLINE KEY("4")

#define print_error(fmt, ...) eprintf(              \
    KEY_BRIGHT_RED KEY_BOLD "error: " KEY_RESET fmt \
    __VA_OPT__(,) __VA_ARGS__                       \
)

#define print_system_error(fmt, ...) eprintf(       \
    KEY_RED KEY_BOLD "system error: " KEY_RESET fmt \
    __VA_OPT__(,) __VA_ARGS__                       \
)

void print_errno(void);

typedef enum result {
    SUCCESS = 0,
    ERROR = -1,
} result_t;

typedef enum error_kind {
    // scanning
    ERROR_UNEXPECTED_CHARACTER,
    ERROR_UNCLOSED_STRING,

    // parsing
    ERROR_UNCLOSED_PARENS,
    ERROR_UNCLOSED_BRACKETS,
    ERROR_UNCLOSED_BRACES,

    ERROR_INVALID_ITEM_DECLARATION,

    ERROR_INTEGER_TOO_BIG,

    ERROR_EXPECTED_BLOCK,
    ERROR_EXPECTED_BLOCK_END,

    ERROR_MISSING_INDEX,
    ERROR_EXCESS_INDEX_TOKENS,

    ERROR_MISSING_EQUALS_IN_BINDING,

    ERROR_MEMBER_NOT_IDENTIFIER,

    ERROR_EXCESS_TOKENS_IN_PARENS,

    ERROR_INCOMPATIBLE_BINARY_OPERATIONS,

    ERROR_INVALID_TYPE_NAME,

    ERROR_VARIABLE_DECLARATION_INVALID_NAME,
    ERROR_VARIABLE_DECLARATION_INVALID_TYPE,

    ERROR_NOT_FUNCTION_SIGNATURE,
    ERROR_UNCLOSED_PARAMETER_LIST, // FIXME: replace with `UNCLOSED_PARENS`

    ERROR_COMPOSITE_MISSING_FIELD_LIST,
    ERROR_COMPOSITE_UNCLOSED_FIELD_LIST, // FIXME: replace with `UNCLOSED_BRACES`
} error_kind_t;

typedef struct error {
    error_kind_t kind;
    text_view_t source;
    string_buf_t message;
} error_t;

typedef struct reporter {
    void(*send)(struct reporter* self, error_t error);
    size_t nerrors;
    // data goes here
} reporter_t;

#define report(reporter, error) do {        \
    (reporter)->send((reporter), (error));  \
    (reporter)->nerrors++;                  \
} while(0)

typedef struct default_reporter {
    reporter_t reporter;
    const source_t* source;
} default_reporter_t;

default_reporter_t new_default_reporter(const source_t* source);
