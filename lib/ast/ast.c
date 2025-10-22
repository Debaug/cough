#include "ast/ast.h"

IMPL_ARRAY_BUF(Expression)

void ast_free(Ast* ast) {
    ast_storage_free(&ast->storage);
}
