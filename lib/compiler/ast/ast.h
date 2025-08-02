#pragma once

#include "compiler/ast/storage.h"
#include "compiler/ast/program.h"
#include "compiler/ast/parser.h"
#include "compiler/ast/analyzer.h"
#include "compiler/ast/debug.h"

typedef struct Ast {
    AstStorage storage;
    Program program;
} Ast;

Result parse(Parser* parser, Ast* dst);
void free_ast(Ast ast);

Result analyze(Analyzer* analyzer, Ast* ast);

void debug_ast(Ast ast, AstDebugger* debugger);
