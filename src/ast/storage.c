#include "ast/storage.h"

ast_storage_t new_ast_storage(void) {
    return (ast_storage_t){
        .arena_stack = new_arena_stack(),
        .alloc_stack = new_alloc_stack(),
    };
}

void free_ast_storage(ast_storage_t storage) {
    free_arena_stack(storage.arena_stack);
    free_alloc_stack(storage.alloc_stack);
}
