#include "ast/ast.h"

parse_result_t parse(parser_t* parser, ast_t* dst) {
    ast_t ast;
    if (parse_program(parser, &ast.program) != PARSE_SUCCESS) {
        return PARSE_ERROR;
    }
    ast.storage = parser->storage;
    *dst = ast;
    return PARSE_SUCCESS;
}

void free_ast(ast_t ast) {
    free_ast_storage(ast.storage);
}

void debug_ast(ast_t ast, ast_debugger_t* debugger) {
    debug_program(ast.program, debugger);
}
