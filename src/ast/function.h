#pragma once

#include "ast/parser.h"
#include "ast/type.h"
#include "ast/expression.h"
#include "ast/debug.h"

typedef struct parameter {
    text_view_t identifier;
    named_type_t type;
} parameter_t;

parse_result_t parse_parameter(parser_t* parser, parameter_t* dst);

void debug_parameter(parameter_t parameter, ast_debugger_t* debugger);

typedef array_buf_t(parameter_t) parameter_array_buf_t;

typedef struct function {
    parameter_array_buf_t parameters;
    bool has_return_type : 1;
    named_type_t return_type;
    block_t body;
} function_t;

parse_result_t parse_function(parser_t* parser, function_t* dst);

void debug_function(function_t function, ast_debugger_t* debugger);
