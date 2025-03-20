#include <stdalign.h>

#include "compiler/diagnostics.h"

static void destroy_simple_compiler_diagnosis(Diagnosis* raw) {
    SimpleCompilerDiagnosis* self = (SimpleCompilerDiagnosis*)raw;
    free_array_buf(self->message);
}

static Severity simple_compiler_diagnosis_severity(const Diagnosis* raw) {
    const SimpleCompilerDiagnosis* self = (SimpleCompilerDiagnosis*)raw;
    return self->severity;
}

static usize simple_compiler_diagnosis_len(const Diagnosis* raw) {
    return 2;
}

static DiagnosisPart simple_compiler_diagnosis_part(const Diagnosis* raw, usize index) {
    const SimpleCompilerDiagnosis* self = (SimpleCompilerDiagnosis*)raw;
    switch (index) {
    case 0:
        return (DiagnosisPart){
            .kind = DIAGNOSIS_PART_MESSAGE,
            .as.message = STRING_VIEW(self->message),
        };
    
    case 1:
        return (DiagnosisPart){
            .kind = DIAGNOSIS_PART_SOURCE_CODE,
            .as.source_code = self->source,
        };

    // invalid
    default:
        exit(-1);
    }
}

static const DiagnosisVTable simple_compiler_diagnosis_vtable = {
    .size = sizeof(SimpleCompilerDiagnosis),
    .alignment = alignof(SimpleCompilerDiagnosis),
    .destroy = destroy_simple_compiler_diagnosis,
    .severity = simple_compiler_diagnosis_severity,
    .len = simple_compiler_diagnosis_len,
    .part = simple_compiler_diagnosis_part,
};

void report_simple_compiler_error(
    Reporter* reporter,
    CompilerErrorKind kind,
    StringBuf message,
    TextView source
) {
    SimpleCompilerDiagnosis diagnosis = {
        .base = {
            .vtable = &simple_compiler_diagnosis_vtable,
        },
        .severity = SEVERITY_ERROR,
        .kind = kind,
        .message = message,
        .source = source,
    };
    report(reporter, &diagnosis.base);
}

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

    usize* line_start_indices = source->line_start_indices.data;
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
            KEY_UNDERLINE KEY_BOLD "%.*s" KEY_RESET "%.*s\n\n",
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
            "%.*s" KEY_UNDERLINE KEY_BOLD "%.*s" KEY_RESET "%.*s\n\n",
            head_len, start_line,
            (int)span.len, span.data,
            tail_len, tail_start
        );
    }
}

static void compiler_reporter_report(Reporter* raw, Diagnosis* diagnosis) {
    CompilerReporter* self = (CompilerReporter*)raw;
    self->n_errors++;

    usize len = diagnosis->vtable->len(diagnosis);
    for (usize i = 0; i < len; i++) {
        DiagnosisPart part = diagnosis->vtable->part(diagnosis, i);
        switch (part.kind) {
        case DIAGNOSIS_PART_MESSAGE:
            log_message(part.as.message, diagnosis->vtable->severity(diagnosis));
            break;
        case DIAGNOSIS_PART_SOURCE_CODE:
            print_error_source_code(self->source, part.as.source_code);
            break;
        }
    }

    diagnosis->vtable->destroy(diagnosis);
}

static usize compiler_reporter_n_errors(const Reporter* raw) {
    CompilerReporter* self = (CompilerReporter*)raw;
    return self->n_errors;
}

ReporterVTable compiler_reporter_vtable = {
    .report = compiler_reporter_report,
    .n_errors = compiler_reporter_n_errors,
};

CompilerReporter new_compiler_reporter(const Source* source) {
    return (CompilerReporter){
        .base = {
            .vtable = &compiler_reporter_vtable,
        },
        .source = source,
        .n_errors = 0
    };
}
