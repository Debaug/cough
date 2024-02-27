#pragma once

#include <stdlib.h>
#include <stdalign.h>

#include "alloc/alloc.h"

typedef struct arena dyn_arena_t;

typedef struct arena_stack {
    dyn_arena_t* top;
} arena_stack_t;

arena_stack_t new_arena_stack(void);
void free_arena_stack(arena_stack_t stack);

void* raw_arena_stack_alloc(arena_stack_t* stack, size_t size, size_t alignment);
#define arena_alloc(arena, elt, ...) \
    raw_arena_stack_alloc((arena), LAYOUT_OF(elt, __VA_ARGS__))

void* raw_arena_stack_extend(
    arena_stack_t* stack,
    void* data,
    size_t size,
    size_t alignment
);
#define arena_stack_push(arena, elt) \
    raw_arena_stack_extend((arena), &(elt), LAYOUT_OF(elt))

typedef struct arena_stack_state {
    dyn_arena_t* top;
    void* end;
} arena_stack_state_t;

arena_stack_state_t arena_stack_snapshot(arena_stack_t stack);
void arena_stack_restore(arena_stack_t* stack, arena_stack_state_t state);
