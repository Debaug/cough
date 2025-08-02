#include "compiler/ast/storage.h"

AstStorage new_ast_storage(void) {
    return (AstStorage){
        .arena_stack = new_arena_stack(),
        .alloc_stack = new_alloc_stack(),
    };
}

void free_ast_storage(AstStorage storage) {
    free_arena_stack(storage.arena_stack);
    free_alloc_stack(storage.alloc_stack);
}
