#pragma once

#include "alloc/arena_stack.h"
#include "alloc/alloc_stack.h"

typedef struct AstStorage {
    ArenaStack arena_stack;
    AllocStack alloc_stack;
} AstStorage;

#define ast_box(storage, val) arena_stack_push(&(storage)->arena_stack, val)
#define ast_push_alloc(storage, val) alloc_stack_push(&(storage)->alloc_stack, val)

AstStorage new_ast_storage(void);
void free_ast_storage(AstStorage storage);
