#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "util/array.h"

typedef struct ast_debugger {
    unsigned int depth;
    bool in_field;
} ast_debugger_t;

ast_debugger_t new_ast_debugger();

void ast_debug_start(ast_debugger_t* debugger, const char* name);
void ast_debug_end(ast_debugger_t* debugger);

void ast_debug_flag(ast_debugger_t* debugger, const char* name);
void ast_debug_key(ast_debugger_t* debugger, const char* name);

void ast_debug_string(ast_debugger_t* debugger, const char* value);
void ast_debug_string_view(ast_debugger_t* debugger, string_view_t value);
void ast_debug_char(ast_debugger_t* debugger, char value);
void ast_debug_int(ast_debugger_t* debugger, intmax_t value);
void ast_debug_uint(ast_debugger_t* debugger, uintmax_t value);
void ast_debug_pointer(ast_debugger_t* debugger, const void* value);

void ast_debug_start_sequence(ast_debugger_t* debugger);
void ast_debug_end_sequence(ast_debugger_t* debugger);
