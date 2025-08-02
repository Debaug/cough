#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "alloc/array.h"

typedef struct AstDebugger {
    unsigned int depth;
    bool in_field;
} AstDebugger;

AstDebugger new_ast_debugger();

void ast_debug_start(AstDebugger* debugger, const char* name);
void ast_debug_end(AstDebugger* debugger);

void ast_debug_flag(AstDebugger* debugger, const char* name);
void ast_debug_key(AstDebugger* debugger, const char* name);

void ast_debug_string(AstDebugger* debugger, const char* value);
void ast_debug_string_view(AstDebugger* debugger, StringView value);
void ast_debug_char(AstDebugger* debugger, char value);
void ast_debug_int(AstDebugger* debugger, imax value);
void ast_debug_uint(AstDebugger* debugger, umax value);
void ast_debug_pointer(AstDebugger* debugger, const void* value);

void ast_debug_start_sequence(AstDebugger* debugger);
void ast_debug_end_sequence(AstDebugger* debugger);
