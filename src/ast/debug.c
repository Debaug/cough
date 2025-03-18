#include <stdio.h>

#include "ast/debug.h"
#include "diagnostic/diagnostic.h"

AstDebugger new_ast_debugger() {
    return (AstDebugger){ .depth = 0, .in_field = false };
}

#define PRINT_INDENTED(debugger, format, ...) \
    eprintf("%*s" format, debugger->depth * 4, "" __VA_OPT__(,) __VA_ARGS__)

#define DEBUG(debugger, format, ...)                                \
    if (debugger->in_field) {                                       \
        eprintf(format __VA_OPT__(,) __VA_ARGS__);             \
        debugger->in_field = false;                                 \
    } else {                                                        \
        PRINT_INDENTED(debugger, format __VA_OPT__(,) __VA_ARGS__); \
    }

void ast_debug_start(AstDebugger* debugger, const char* name) {
    DEBUG(debugger, "%s (\n", name);
    debugger->depth++;
}

void ast_debug_end(AstDebugger* debugger) {
    debugger->depth--;
    PRINT_INDENTED(debugger, ")\n");
}

void ast_debug_flag(AstDebugger* debugger, const char* name) {
    PRINT_INDENTED(debugger, "%s\n", name);
}

void ast_debug_key(AstDebugger* debugger, const char* name) {
    PRINT_INDENTED(debugger, "%s: ", name);
    debugger->in_field = true;
}

void ast_debug_string(AstDebugger* debugger, const char* value) {
    DEBUG(debugger, "'%s'\n", value);
}

void ast_debug_string_view(AstDebugger* debugger, StringView value) {
    DEBUG(debugger, "'%.*s'\n", (int)value.len, value.data);
}

void ast_debug_char(AstDebugger* debugger, char value) {
    DEBUG(debugger, "'%c'\n", value);
}

void ast_debug_int(AstDebugger* debugger, imax value) {
    DEBUG(debugger, "%jd\n", value);
}

void ast_debug_uint(AstDebugger* debugger, umax value) {
    DEBUG(debugger, "%jd\n", value);
}

void ast_debug_pointer(AstDebugger* debugger, const void* value) {
    DEBUG(debugger, "%p\n", value);
}

void ast_debug_start_sequence(AstDebugger* debugger) {
    DEBUG(debugger, "[\n");
    debugger->depth++;
}

void ast_debug_end_sequence(AstDebugger* debugger) {
    debugger->depth--;
    PRINT_INDENTED(debugger, "]\n");
}
