#include "ast/ast.h"

Result parse(Parser* parser, Ast* dst) {
    Ast ast;
    if (parse_program(parser, &ast.program) != SUCCESS) {
        return ERROR;
    }
    ast.storage = parser->storage;
    *dst = ast;
    return SUCCESS;
}

void free_ast(Ast ast) {
    free_ast_storage(ast.storage);
}

void debug_ast(Ast ast, AstDebugger* debugger) {
    debug_program(ast.program, debugger);
}
