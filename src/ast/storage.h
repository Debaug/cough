#pragma once

#include "alloc/arena_stack.h"
#include "alloc/alloc_stack.h"

typedef struct ast_storage {
    arena_stack_t arena_stack;
    alloc_stack_t alloc_stack;
} ast_storage_t;

#define ast_box(storage, val) arena_stack_push(&(storage)->arena_stack, val)
#define ast_push_alloc(storage, val) ast_push_alloc(&(storage)->alloc_stack, val)

ast_storage_t new_ast_storage(void);
void free_ast_storage(ast_storage_t storage);
