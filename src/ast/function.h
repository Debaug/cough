#pragma once

#include "ast/parser.h"
#include "ast/type.h"
#include "ast/expression.h"
#include "ast/debug.h"

typedef struct function {
    variable_array_buf_t parameters;
    bool has_return_type : 1;
    named_type_t return_type;
    block_t body;
} function_t;

parse_result_t parse_function(parser_t* parser, function_t* dst);

void debug_function(function_t function, ast_debugger_t* debugger);
