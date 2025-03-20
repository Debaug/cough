#pragma once

#include "compiler/ast/parser.h"
#include "compiler/ast/analyzer.h"
#include "compiler/ast/type.h"
#include "compiler/ast/expression.h"
#include "compiler/ast/debug.h"

typedef struct Function {
    FunctionSignature signature;
    Block body;
    Scope* scope;
} Function;

Result parse_function(Parser* parser, Function* dst);

Result analyze_function(Analyzer* analyzer, Function* function);

void debug_parameter(Variable parameter, AstDebugger* debugger);
void debug_function(Function function, AstDebugger* debugger);
