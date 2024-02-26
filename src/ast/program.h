#pragma once

#include "ast/function.h"
#include "ast/parser.h"
#include "ast/parse_result.h"
#include "ast/debug.h"

#include "util/array.h"

typedef enum item_declaration_kind {
    FUNCTION_DECLARATION,
} top_declaration_kind_t;

typedef struct item_declaration {
    text_view_t identifier;
    top_declaration_kind_t kind;
    union {
        function_t function;
    } as;
} item_declaration_t;

parse_error_t parse_item_declaration(parser_t* parser, item_declaration_t* dst);

typedef array_buf_t(item_declaration_t) item_declaration_array_buf_t;
typedef struct program {
    item_declaration_array_buf_t item_declarations;
} program_t;

parse_error_t parse_program(parser_t* parser, program_t* program);

void debug_program(
    program_t program,
    ast_storage_t storage,
    ast_debugger_t* debugger
);
void debug_item_declaration(
    item_declaration_t item_declaration,
    ast_storage_t storage,
    ast_debugger_t* debugger
);
