#include <string.h>
#include <errno.h>

#include "diagnostics/diagnostics.h"

void print_errno(void) {
    print_system_error("%s (errno %d)", strerror(errno), errno);
}

void report_start(Reporter* reporter, Severity severity, int code) {
    (reporter->vtable->start)(reporter, severity, code);
}

void report_end(Reporter* reporter) {
    (reporter->vtable->end)(reporter);
}

void report_message(Reporter* reporter, StringBuf message) {
    (reporter->vtable->message)(reporter, message);
}

void report_source_code(Reporter* reporter, Range source_code) {
    (reporter->vtable->source_code)(reporter, source_code);
}

usize reporter_error_count(const Reporter* reporter) {
    return (reporter->vtable->error_count)(reporter);
}
