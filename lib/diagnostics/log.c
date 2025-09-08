#include "diagnostics/log.h"

#define KEY(n) "\x1B[" #n "m"
#define KEY_RESET KEY(0)
#define KEY_BOLD KEY(1)
#define KEY_RED KEY(31)
#define KEY_BRIGHT_RED KEY(91)

const char* severity_prefix(Severity severity) {
    switch (severity) {
    case SEVERITY_ERROR: return KEY_BRIGHT_RED "error: " KEY_RESET;
    case SEVERITY_SYSTEM_ERROR: return KEY_RED "system error: " KEY_RESET;
    default: return KEY_RED "unknown severity: " KEY_RESET;
    }
}
