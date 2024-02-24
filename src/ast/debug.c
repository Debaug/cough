#include <stdio.h>

#include "ast/debug.h"
#include "diagnostic/diagnostic.h"

ast_debugger_t new_ast_debugger() {
    return (ast_debugger_t){ .depth = 0, .in_field = false };
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

void ast_debug_start(ast_debugger_t* debugger, const char* name) {
    DEBUG(debugger, "%s (\n", name);
    debugger->depth++;
}

void ast_debug_end(ast_debugger_t* debugger) {
    debugger->depth--;
    PRINT_INDENTED(debugger, ")\n");
}

void ast_debug_flag(ast_debugger_t* debugger, const char* name) {
    PRINT_INDENTED(debugger, "%s\n", name);
}

void ast_debug_key(ast_debugger_t* debugger, const char* name) {
    PRINT_INDENTED(debugger, "%s: ", name);
    debugger->in_field = true;
}

void ast_debug_string(ast_debugger_t* debugger, const char* value) {
    DEBUG(debugger, "'%s'\n", value);
}

void ast_debug_string_view(ast_debugger_t* debugger, string_view_t value) {
    DEBUG(debugger, "'%.*s'\n", (int)value.len, value.ptr);
}

void ast_debug_char(ast_debugger_t* debugger, char value) {
    DEBUG(debugger, "'%c'\n", value);
}

void ast_debug_int(ast_debugger_t* debugger, intmax_t value) {
    DEBUG(debugger, "%jd\n", value);
}

void ast_debug_uint(ast_debugger_t* debugger, uintmax_t value) {
    DEBUG(debugger, "%jd\n", value);
}

void ast_debug_pointer(ast_debugger_t* debugger, const void* value) {
    DEBUG(debugger, "%p\n", value);
}

void ast_debug_start_sequence(ast_debugger_t* debugger) {
    DEBUG(debugger, "[\n");
    debugger->depth++;
}

void ast_debug_end_sequence(ast_debugger_t* debugger) {
    debugger->depth--;
    PRINT_INDENTED(debugger, "]\n");
}
