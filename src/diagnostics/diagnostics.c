#include <string.h>
#include <errno.h>

#include "diagnostics/diagnostics.h"

void log_message(StringView message, Severity severity) {
    switch (severity) {
    case SEVERITY_ERROR:
        print_error("%.*s", (int)message.len, message.data);
    case SEVERITY_SYSTEM_ERROR:
        print_system_error("%.*s", (int)message.len, message.data);
    }
}

void print_errno(void) {
    print_system_error("%s (errno %d)", strerror(errno), errno);
}

void report(Reporter* reporter, Diagnosis* diagnosis) {
    reporter->vtable->report(reporter, diagnosis);
}
