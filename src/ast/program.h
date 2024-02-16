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

DEFINE_PARSE_RESULT(parse_item_declaration_result, item_declaration_t)
parse_item_declaration_result_t parse_item_declaration(parser_t* parser);

void debug_item_declaration(item_declaration_t item_declaration, ast_debugger_t* debugger);

typedef struct program {
    array_buf_t /* item_declaration_t */ item_declarations;
} program_t;

DEFINE_PARSE_RESULT(parse_program_result, program_t);
parse_program_result_t parse_program(parser_t* parser);

void debug_program(program_t program, ast_debugger_t* debugger);
