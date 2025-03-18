#pragma once

#include "ast/storage.h"
#include "ast/program.h"
#include "ast/parser.h"
#include "ast/analyzer.h"
#include "ast/debug.h"

typedef struct Ast {
    AstStorage storage;
    Program program;
} Ast;

Result parse(Parser* parser, Ast* dst);
void free_ast(Ast ast);

Result analyze(Analyzer* analyzer, Ast* ast);

void debug_ast(Ast ast, AstDebugger* debugger);
