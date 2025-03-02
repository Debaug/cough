#pragma once

#include "ast/parser.h"
#include "ast/analyzer.h"
#include "ast/type.h"
#include "ast/expression.h"
#include "ast/debug.h"

typedef struct function {
    function_signature_t signature;
    block_t body;
    scope_t* scope;
} function_t;

result_t parse_function(parser_t* parser, function_t* dst);

result_t analyze_function(analyzer_t* analyzer, function_t* function);

void debug_parameter(variable_t parameter, ast_debugger_t* debugger);
void debug_function(function_t function, ast_debugger_t* debugger);
