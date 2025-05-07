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

typedef enum Severity {
    SEVERITY_ERROR,
    SEVERITY_SYSTEM_ERROR,
} Severity;

void log_message(StringView message, Severity severity);

typedef struct DiagnosisVTable DiagnosisVTable;

typedef struct Diagnosis {
    const DiagnosisVTable* vtable;
    // data goes here
} Diagnosis;

typedef enum DiagnosisPartKind {
    DIAGNOSIS_PART_MESSAGE,
    DIAGNOSIS_PART_SOURCE_CODE,
} DiagnosisPartKind;

typedef struct DiagnosisPart {
    DiagnosisPartKind kind;
    union {
        StringView message;
        TextView source_code;
    } as;
} DiagnosisPart;

typedef struct DiagnosisVTable {
    usize size;
    usize alignment;
    void(*destroy)(Diagnosis* self);
    Severity(*severity)(const Diagnosis* self);
    usize(*len)(const Diagnosis* self);
    DiagnosisPart(*part)(const Diagnosis* self, usize index);
} DiagnosisVTable;

typedef struct ReporterVTable ReporterVTable;

/// @brief The reporter type.
typedef struct Reporter {
    const ReporterVTable* vtable;
    // data goes here
} Reporter;

typedef struct ReporterVTable {
    // Takes ownership of the diagnosis. This function does not deallocate
    // the memory where `diagnosis` itself is stored though.
    void(*report)(Reporter* self, Diagnosis* diagnosis);
    usize(*n_errors)(const Reporter* self);
} ReporterVTable;

// Takes ownership of the diagnosis. This function does not deallocate
// the memory where `diagnosis` itself is stored though.
void report(Reporter* reporter, Diagnosis* diagnosis);
