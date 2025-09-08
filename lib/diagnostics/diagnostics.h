#pragma once

#include <stdio.h>

#include "collections/array.h"
#include "collections/string.h"

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

typedef enum Severity {
    SEVERITY_ERROR,
    SEVERITY_SYSTEM_ERROR,
} Severity;

typedef struct Reporter {
    const struct ReporterVTable* vtable;
} Reporter;

typedef struct ReporterVTable {
    void(*start)(Reporter* self, Severity severity, int code);
    void(*end)(Reporter* self);
    void(*message)(Reporter* self, StringBuf message);
    void(*source_code)(Reporter* self, Range source_code);
    usize(*error_count)(const Reporter* self);
} ReporterVTable;

void report_start(Reporter* reporter, Severity severity, int code);
void report_end(Reporter* reporter);
void report_message(Reporter* reporter, StringBuf message);
void report_source_code(Reporter* reporter, Range source_code);
usize reporter_error_count(const Reporter* reporter);
