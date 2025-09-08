#pragma once

#include <stdio.h>

/// @brief Formats and prints a message to stderr.
///
/// The arguments follow the same format as `printf`.
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

typedef enum Severity {
    SEVERITY_ERROR,
    SEVERITY_SYSTEM_ERROR,
} Severity;

const char* severity_prefix(Severity severity);
#define log_diagnostic(severity, fmt, ...)  \
    eprintf("%s" fmt "\n", severity_prefix(severity) __VA_OPT__(,) __VA_ARGS__)

#define log_system_error(...) log_diagnostic(SEVERITY_SYSTEM_ERROR, __VA_ARGS__)
#define log_error(...) log_diagnostic(SEVERITY_ERROR, __VA_ARGS__)
