#include <stdalign.h>

#include "vm/diagnostics.h"

static void runtime_report_start(Reporter* raw, Severity severity, int code) {
    RuntimeReporter* reporter = (RuntimeReporter*)raw;
    reporter->n_errors++;
    switch (severity) {
    case SEVERITY_ERROR: print_error(""); break;
    case SEVERITY_SYSTEM_ERROR: print_system_error(""); break;
    }
}

static void runtime_report_end(Reporter* raw) {
    eprintf("\n");
}

static void runtime_report_message(Reporter* raw, StringBuf message) {
    eprintf("%.*s\n", STRING_FMT(message));
}

static void runtime_report_source_code(Reporter* raw, TextView span) {
    if (span.start.line == span.end.line) {
        eprintf(
            "at %zu.%zu-%zu\n",
            span.start.line + 1,
            span.start.column + 1,
            span.end.column + 1
        );
    } else {
        eprintf(
            "at %zu.%zu-%zu.%zu\n",
            span.start.line + 1,
            span.start.column + 1,
            span.end.line + 1,
            span.end.column + 1
        );
    }
}

static usize runtime_report_n_errors(const Reporter* raw) {
    const RuntimeReporter* reporter = (RuntimeReporter*)raw;
    return reporter->n_errors;
}

static const ReporterVTable runtime_reporter_vtable = {
    .start = runtime_report_start,
    .end = runtime_report_end,
    .message = runtime_report_message,
    .source_code = runtime_report_source_code,
    .n_errors = runtime_report_n_errors,
};

RuntimeReporter new_runtime_reporter(void) {
    return (RuntimeReporter){
        .base.vtable = &runtime_reporter_vtable,
        .n_errors = 0,
    };
}

void report_simple_runtime_error(
    Reporter* reporter,
    RuntimeErrorKind kind,
    StringBuf message
) {
    report_start(reporter, SEVERITY_ERROR, kind);
    report_message(reporter, message);
    report_end(reporter);
}
