#include <stdalign.h>

#include "vm/diagnostics.h"

static void runtime_report_start(Reporter* raw, Severity severity, i32 code) {
    RuntimeReporter* reporter = (RuntimeReporter*)raw;
    reporter->error_count++;
    reporter->severity = severity;
}

static void runtime_report_end(Reporter* raw) {
    eprintf("\n");
}

static void runtime_report_message(Reporter* raw, StringBuf message) {
    RuntimeReporter* reporter = (RuntimeReporter*)raw;
    log_diagnostic(reporter->severity, "%.*s\n", (int)message.len, message.data);
}

static void runtime_report_source_code(Reporter* raw, Range span) {
    eprintf("at [%zu]-[%zu]\n", span.start, span.end);
}

static usize runtime_report_error_count(const Reporter* raw) {
    const RuntimeReporter* reporter = (RuntimeReporter*)raw;
    return reporter->error_count;
}

static const ReporterVTable runtime_reporter_vtable = {
    .start = runtime_report_start,
    .end = runtime_report_end,
    .message = runtime_report_message,
    .source_code = runtime_report_source_code,
    .error_count = runtime_report_error_count,
};

RuntimeReporter new_runtime_reporter(void) {
    return (RuntimeReporter){
        .base.vtable = &runtime_reporter_vtable,
        .error_count = 0,
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
