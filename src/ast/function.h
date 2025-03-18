#pragma once

#include "ast/parser.h"
#include "ast/analyzer.h"
#include "ast/type.h"
#include "ast/expression.h"
#include "ast/debug.h"

typedef struct Function {
    FunctionSignature signature;
    Block body;
    Scope* scope;
} Function;

Result parse_function(Parser* parser, Function* dst);

Result analyze_function(Analyzer* analyzer, Function* function);

void debug_parameter(Variable parameter, AstDebugger* debugger);
void debug_function(Function function, AstDebugger* debugger);
