#pragma once

#include "ast/parser.h"
#include "ast/parse_result.h"
#include "ast/type.h"
#include "ast/expression.h"
#include "ast/debug.h"

typedef struct parameter {
    text_view_t identifier;
    named_type_t type;
} parameter_t;

DEFINE_PARSE_RESULT(parse_parameter_result, parameter_t);
parse_parameter_result_t parse_parameter(parser_t* parser);

void debug_parameter(parameter_t parameter, ast_debugger_t* debugger);

typedef struct function {
    array_buf_t /* parameter_t */ parameters;
    bool has_return_type : 1;
    named_type_t return_type;
    block_t body;
} function_t;

DEFINE_PARSE_RESULT(parse_function_result, function_t);
parse_function_result_t parse_function(parser_t* parser);

void debug_function(function_t function, ast_debugger_t* debugger);
