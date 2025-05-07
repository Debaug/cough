#include <stdalign.h>

#include "vm/diagnostics.h"

static void runtime_reporter_report(Reporter* raw, Diagnosis* diagnosis) {
    RuntimeReporter* self = (RuntimeReporter*)raw;

    usize len = diagnosis->vtable->len(diagnosis);
    Severity severity = diagnosis->vtable->severity(diagnosis);
    for (usize i = 0; i < len; i++) {
        DiagnosisPart part = diagnosis->vtable->part(diagnosis, i);
        switch (part.kind) {
        case DIAGNOSIS_PART_SOURCE_CODE: continue;
        case DIAGNOSIS_PART_MESSAGE:
            log_message(part.as.message, severity);
            break;
        }
    }

    diagnosis->vtable->destroy(diagnosis);
}

static usize runtime_reporter_n_errors(const Reporter* raw) {
    const RuntimeReporter* self = (const RuntimeReporter*)raw;
    return self->n_errors;
}

static const ReporterVTable runtime_reporter_vtable = {
    .report = runtime_reporter_report,
    .n_errors = runtime_reporter_n_errors
};

RuntimeReporter new_runtime_reporter(void) {
    return (RuntimeReporter){
        .base.vtable = &runtime_reporter_vtable,
    };
}
typedef struct SimpleRuntimeError {
    Diagnosis base;
    StringBuf message;
} SimpleRuntimeError;

static void destroy_simple_runtime_error(Diagnosis* raw) {
    SimpleRuntimeError* self = (SimpleRuntimeError*)raw;
    free_array_buf(self->message);
}

static Severity simple_runtime_error_severity(const Diagnosis* raw) {
    return SEVERITY_ERROR;
}

static usize simple_runtime_error_len(const Diagnosis* raw) {
    return 1;
}

static DiagnosisPart simple_runtime_error_part(const Diagnosis* raw, usize index) {
    const SimpleRuntimeError* self = (const SimpleRuntimeError*)raw;
    return (DiagnosisPart){
        .kind = DIAGNOSIS_PART_MESSAGE,
        .as.message = STRING_VIEW(self->message),
    };
}

static const DiagnosisVTable simple_runtime_error_vtable = {
    .size = sizeof(SimpleRuntimeError),
    .alignment = alignof(SimpleRuntimeError),
    .destroy = destroy_simple_runtime_error,
    .severity = simple_runtime_error_severity,
    .len = simple_runtime_error_len,
    .part = simple_runtime_error_part,
};

void report_simple_runtime_error(
    Reporter* reporter,
    RuntimeErrorKind kind,
    StringBuf message
) {
    SimpleRuntimeError diagnosis = {
        .base.vtable = &simple_runtime_error_vtable,
        .message = message,
    };
    reporter->vtable->report(reporter, &diagnosis.base);
}
