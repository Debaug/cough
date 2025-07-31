#include <stdalign.h>

#include "compiler/diagnostics.h"

static void print_error_source_code(const Source* source, TextView span) {
    bool snip;
    bool single_line;
    if (span.start.line == span.end.line) {
        snip = false;
        single_line = true;
    } else if (span.start.line - span.end.line < 3) {
        snip = false;
        single_line = false;
    } else {
        snip = true;
        single_line = false;
    }

    if (single_line) {
        eprintf(
            "at %s:%zu.%zu-%zu\n",
            source->path,
            span.start.line + 1,
            span.start.column + 1,
            span.end.column + 1
        );
    } else {
        eprintf(
            "at %s:%zu.%zu-%zu.%zu\n",
            source->path,
            span.start.line + 1,
            span.start.column + 1,
            span.end.line + 1,
            span.end.column + 1
        );
    }

    const usize* line_start_indices = source->line_start_indices.data;
    if (snip) {
        const char* first_line = source->text.data + line_start_indices[span.start.line];
        int head_len = span.start.column;
        int first_line_len = line_start_indices[span.start.line + 1] - line_start_indices[span.start.line];
        int first_line_error_len = first_line_len - head_len;

        const char* last_line = source->text.data + line_start_indices[span.end.line];
        int last_line_error_len = span.end.column;
        int last_line_len = line_start_indices[span.end.line + 1] - line_start_indices[span.end.line];
        int tail_len = last_line_len - last_line_error_len;

        eprintf(
            "%.*s" KEY_UNDERLINE KEY_BOLD "%.*s\n"
            KEY_RESET "...\n"
            KEY_UNDERLINE KEY_BOLD "%.*s" KEY_RESET "%.*s",
            head_len, first_line, first_line_error_len, first_line + head_len,
            last_line_error_len, last_line, tail_len, last_line + last_line_error_len
        );
    } else {
        const char* start_line = source->text.data + line_start_indices[span.start.line];
        int head_len = span.start.column;
        const char* last_line = source->text.data + line_start_indices[span.end.line];
        const char* tail_start = last_line + span.end.column;
        int tail_len = line_start_indices[span.end.line + 1] - span.end.index;

        eprintf(
            "%.*s" KEY_UNDERLINE KEY_BOLD "%.*s" KEY_RESET "%.*s",
            head_len, start_line,
            (int)span.len, span.data,
            tail_len, tail_start
        );
    }
}

static void compiler_report_start(Reporter* raw, Severity severity, int code) {
    CompilerReporter* reporter = (CompilerReporter*)raw;
    reporter->n_errors++;
    switch (severity) {
    case SEVERITY_ERROR: print_error(""); break;
    case SEVERITY_SYSTEM_ERROR: print_system_error(""); break;
    }
}

static void compiler_report_end(Reporter* raw) {
    eprintf("\n");
}

static void compiler_report_message(Reporter* raw, StringBuf message) {
    eprintf("%.*s\n", STRING_FMT(message));
}

static void compiler_report_source_code(Reporter* raw, TextView source_code) {
    CompilerReporter* reporter = (CompilerReporter*)raw;
    print_error_source_code(reporter->source, source_code);
}

static usize compiler_report_n_errors(const Reporter* raw) {
    const CompilerReporter* reporter = (CompilerReporter*)raw;
    return reporter->n_errors;
}

static const ReporterVTable compiler_reporter_vtable = {
    .start = compiler_report_start,
    .end = compiler_report_end,
    .message = compiler_report_message,
    .source_code = compiler_report_source_code,
    .n_errors = compiler_report_n_errors,
};

CompilerReporter new_compiler_reporter(const Source* source) {
    return (CompilerReporter){
        .base.vtable = &compiler_reporter_vtable,
        .source = source,
        .n_errors = 0
    };
}

void report_simple_compiler_error(
    Reporter* reporter,
    CompilerErrorKind kind,
    StringBuf message,
    TextView source
) {
    report_start(reporter, SEVERITY_ERROR, kind);
    report_message(reporter, message);
    report_source_code(reporter, source);
    report_end(reporter);
}
