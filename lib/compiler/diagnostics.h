#pragma once

#include "diagnostics/diagnostics.h"

typedef enum CompilerErrorKind {
    // scanning

    CE_INVALID_TOKEN,

    // parsing

    CE_UNCLOSED_PARENS,
    CE_UNCLOSED_BRACKETS,
    CE_UNCLOSED_BRACES,

    CE_INTEGER_TOO_BIG,

    CE_INVALID_ITEM_DECLARATION,

    CE_EXPECTED_BLOCK,

    CE_MISSING_EXPRESSION,
    CE_EXCESS_EXPRESSION_TOKENS,

    CE_MEMBER_NOT_IDENTIFIER,

    CE_INCOMPATIBLE_BINARY_OPERATIONS,

    CE_INVALID_TYPE_NAME,

    CE_VARIABLE_DECLARATION_INVALID_NAME,
    CE_VARIABLE_DECLARATION_INVALID_TYPE,

    CE_NOT_FUNCTION_SIGNATURE,

    CE_COMPOSITE_MISSING_FIELD_LIST,

    // analyzing

    CE_DUPLICATE_SYMBOL_NAME,
} CompilerErrorKind;

typedef struct CompilerReporter {
    Reporter base;
    const Source* source;
    usize error_count;
} CompilerReporter;

CompilerReporter new_compiler_reporter(const Source* source);

void report_simple_compiler_error(
    Reporter* reporter,
    CompilerErrorKind kind,
    StringBuf message,
    TextView source
);
