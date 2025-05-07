#include <string.h>
#include <errno.h>

#include "diagnostics/diagnostics.h"

void log_message(StringView message, Severity severity) {
    switch (severity) {
    case SEVERITY_ERROR:
        print_error("%.*s\n", STRING_FMT(message));
        break;
    case SEVERITY_SYSTEM_ERROR:
        print_system_error("%.*s\n", STRING_FMT(message));
        break;
    }
}

void print_errno(void) {
    print_system_error("%s (errno %d)", strerror(errno), errno);
}

void report(Reporter* reporter, Diagnosis* diagnosis) {
    reporter->vtable->report(reporter, diagnosis);
}
