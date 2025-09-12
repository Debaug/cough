#include "diagnostics/crashing.h"
#include "diagnostics/log.h"

static void crashing_report_start(Reporter* raw, Severity severity, i32 code) {
    CrashingReporter* self = (CrashingReporter*)raw;
    self->severity = severity;
}

static void crashing_report_end(Reporter* raw) {
    *(volatile int*)NULL = 0;
    exit(-1);
}

static void crashing_report_message(Reporter* raw, StringBuf message) {
    CrashingReporter* self = (CrashingReporter*)raw;
    log_diagnostic(self->severity, "%.*s", (int)message.len, message.data);
}

static void crashing_report_souce_code(Reporter* raw, Range source) {
    CrashingReporter* self = (CrashingReporter*)raw;
    LineColumn start = source_text_position(self->source, source.start);
    char const* path = self->source.path ? self->source.path : "<stdin>";
    eprintf(
        "at %s:%zu:%zu (%zu): %.*s\n",
        path, start.line + 1, start.column + 1, source.start,
        (int)(source.end - source.start),
        self->source.text + source.start
    );
}

static usize crashing_report_error_count(Reporter const* raw) {
    return 0;
}

static ReporterVTable const crashing_reporter_vtable = {
    .start = crashing_report_start,
    .end = crashing_report_end,
    .message = crashing_report_message,
    .source_code = crashing_report_souce_code,
    .error_count = crashing_report_error_count,
};

CrashingReporter crashing_reporter_new(SourceText source) {
    return (CrashingReporter){
        .base.vtable = &crashing_reporter_vtable,
        .source = source,
        .severity = 0,
    };
}
