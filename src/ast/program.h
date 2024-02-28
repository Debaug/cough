#pragma once

#include "ast/function.h"
#include "ast/parser.h"
#include "ast/debug.h"

#include "alloc/array.h"

typedef enum item_declaration_kind {
    ITEM_FUNCTION,
    ITEM_STRUCT,
    ITEM_VARIANT,
} item_declaration_kind_t;

typedef struct item_declaration {
    text_view_t identifier;
    item_declaration_kind_t kind;
    union {
        function_t function;
        composite_type_t composite;
    } as;
} item_declaration_t;

typedef array_buf_t(item_declaration_t) item_declaration_array_buf_t;
typedef struct program {
    item_declaration_array_buf_t item_declarations;
} program_t;

parse_result_t parse_item_declaration(parser_t* parser, item_declaration_t* dst);
parse_result_t parse_program(parser_t* parser, program_t* program);

void debug_program(program_t program, ast_debugger_t* debugger);
void debug_item_declaration(
    item_declaration_t item_declaration,
    ast_debugger_t* debugger
);
