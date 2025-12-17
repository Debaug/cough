#include "ast/ast.h"

void ast_free(Ast* ast) {
    ast_storage_free(&ast->storage);
}
