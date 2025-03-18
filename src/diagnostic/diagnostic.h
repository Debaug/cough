#pragma once

#include <stdio.h>

#include "text/text.h"
#include "alloc/array.h"

/// @brief Formats and prints a message to stderr.
///
/// The arguments follow the same format as `printf`.
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// Keys for formatting diagnostic text.

#define KEY(n) "\x1B[" n "m"
#define KEY_RESET KEY("0")
#define KEY_BOLD KEY("1")
#define KEY_RED KEY("31")
#define KEY_BRIGHT_RED KEY("91")
#define KEY_BRIGHT_CYAN KEY("96")
#define KEY_UNDERLINE KEY("4")

/// @brief Prints a 'regular' error.
///
/// The error message is preceded by a brighter red-colored 'error: ' string.
/// This format of errors is mainly intended for compilations errors, *not* internal
/// errors or errors emitted while running a Cough program.
///
/// The arguments follow the same format as `printf`, but the format string must
/// be a literal.
#define print_error(fmt, ...) eprintf(              \
    KEY_BRIGHT_RED KEY_BOLD "error: " KEY_RESET fmt \
    __VA_OPT__(,) __VA_ARGS__                       \
)

/// @brief Prints a 'system' error.
///
/// The error message is preceded by a red-colored 'system error: ' string.
/// This format of errors is intended for internal errors, i. e. those caused by a
/// memory allocation or file system I/O. It is not intended to report errors due
/// to incorrect Cough code.
///
/// The arguments follow the same format as `printf`, but the format string must
/// be a literal.
#define print_system_error(fmt, ...) eprintf(       \
    KEY_RED KEY_BOLD "system error: " KEY_RESET fmt \
    __VA_OPT__(,) __VA_ARGS__                       \
)

/// @brief Reports a system error from `errno`.
///
/// This function prints the associated message, which is provided by the C
/// standard library, as well as the value of `errno` itself.
void print_errno(void);

/// @brief The result type.
typedef enum Result {
    /// @brief Indicates the callee has returned properly and that the caller
    /// may continue normally.
    ///
    /// This does not mean that no error has been emitted -- rather, that all
    /// errors have been contained and that the current stage can continue like
    /// no errors have occured. However, if an error has occured (which is
    /// recorded by the reporter), the next stage may not be executed.
    SUCCESS = 0,

    /// @brief Indicates that the callee has failed, and that the caller may not
    /// continue executing right away.
    ///
    /// For example, a parsing function that has returned `ERROR` might have
    /// left the parser in the middle of e. g. an expression, and that the remaining
    /// token stream does not start with valid Cough code.
    ///
    /// The caller is then responsible for putting the stage back into a state
    /// where it can continue, or to propagate the `ERROR` upstream.
    ///
    /// Regardless of success or failure, the *callee* is responsible for managing
    /// its resources like allocated memory.
    ERROR = -1,
} Result;

typedef enum ErrorKind {
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

    // analyzing

    ERROR_DUPLICATE_SYMBOL_NAME,

    // runtime

    ERROR_INVALID_INSTRUCTION
} ErrorKind;

typedef enum Severity {
    SEVERITY_VERBOSE,
    SEVERITY_DEBUG,
    SEVERITY_INFO,
    SEVERITY_WARNING,
    SEVERITY_ERROR,
    SEVERITY_SYSTEM_ERROR,
} Severity;

typedef struct DiagnosisVTable DiagnosisVTable;

typedef struct Diagnosis {
    const DiagnosisVTable* vtable;
    // data goes here
} Diagnosis;

typedef struct DiagnosisVTable {
    void(*free)(Diagnosis* self);
} DiagnosisVTable;

/// @brief The error type.
typedef struct Error {
    ErrorKind kind;
    TextView source;
    StringBuf message;
} Error;

/// @brief The error reporter type.
typedef struct Reporter {
    // takes ownership of the error
    void(*send)(struct Reporter* self, Error* error);
    usize nerrors;
    // data goes here
} Reporter;

/// @brief Reports the given error to the given reporter.
#define report(reporter, error) do {        \
    (reporter)->send((reporter), &(error)); \
    (reporter)->nerrors++;                  \
} while(0)

/// @brief A general-purpose reporter that prints errors to stderr.
typedef struct DefaultReporter {
    Reporter reporter;
    const Source* source;
} DefaultReporter;

/// @brief Constructs a @ref DefaultReporter.
DefaultReporter new_default_reporter(const Source* source);
