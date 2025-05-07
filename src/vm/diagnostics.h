#pragma once

#include "diagnostics/diagnostics.h"

typedef enum RuntimeErrorKind {
    RE_INVALID_INSTRUCTION,
    RE_INVALID_SYSCALL,
    RE_INTEGER_OVERFLOW,
} RuntimeErrorKind;

typedef struct RuntimeReporter {
    Reporter base;
    usize n_errors;
} RuntimeReporter;

RuntimeReporter new_runtime_reporter(void);

void report_simple_runtime_error(
    Reporter* reporter,
    RuntimeErrorKind kind,
    StringBuf message
);
