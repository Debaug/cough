#include <string.h>
#include <errno.h>

#include "diagnostic/diagnostic.h"

void report_errno(void) {
    report_system_error("%s (errno %d)", strerror(errno), errno);
}
