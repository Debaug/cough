#pragma once

#include "ast/storage.h"
#include "ast/program.h"
#include "ast/parser.h"
#include "ast/analyzer.h"
#include "ast/debug.h"

typedef struct ast {
    ast_storage_t storage;
    program_t program;
} ast_t;

result_t parse(parser_t* parser, ast_t* dst);
void free_ast(ast_t ast);

analyze_result_t analyze(analyzer_t* analyzer, ast_t* ast);

void debug_ast(ast_t ast, ast_debugger_t* debugger);
