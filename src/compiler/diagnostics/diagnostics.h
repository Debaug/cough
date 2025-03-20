#pragma once

#include "diagnostics/diagnostics.h"

typedef enum CompilerErrorKind {
    // scanning

    ERROR_INVALID_TOKEN,

    // parsing

    ERROR_UNCLOSED_PARENS,
    ERROR_UNCLOSED_BRACKETS,
    ERROR_UNCLOSED_BRACES,

    ERROR_INTEGER_TOO_BIG,

    ERROR_INVALID_ITEM_DECLARATION,

    ERROR_EXPECTED_BLOCK,

    ERROR_MISSING_EXPRESSION,
    ERROR_EXCESS_EXPRESSION_TOKENS,

    ERROR_MEMBER_NOT_IDENTIFIER,

    ERROR_INCOMPATIBLE_BINARY_OPERATIONS,

    ERROR_INVALID_TYPE_NAME,

    ERROR_VARIABLE_DECLARATION_INVALID_NAME,
    ERROR_VARIABLE_DECLARATION_INVALID_TYPE,

    ERROR_NOT_FUNCTION_SIGNATURE,

    ERROR_COMPOSITE_MISSING_FIELD_LIST,

    // analyzing

    ERROR_DUPLICATE_SYMBOL_NAME,
} CompilerErrorKind;

typedef struct SimpleCompilerDiagnosis {
    Diagnosis base;
    Severity severity;
    CompilerErrorKind kind;
    StringBuf message;
    TextView source;
} SimpleCompilerDiagnosis;

typedef struct CompilerReporter {
    Reporter base;
    const Source* source;
    usize n_errors;
} CompilerReporter;

CompilerReporter new_compiler_reporter(const Source* source);

void report_simple_compiler_error(
    Reporter* reporter,
    CompilerErrorKind kind,
    StringBuf message,
    TextView source
);
