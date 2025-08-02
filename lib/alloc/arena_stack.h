#pragma once

#include <stdlib.h>
#include <stdalign.h>

#include "alloc/alloc.h"

typedef struct DynArena DynArena;

typedef struct ArenaStack {
    DynArena* top;
} ArenaStack;

ArenaStack new_arena_stack(void);
void free_arena_stack(ArenaStack stack);

void* raw_arena_stack_alloc(ArenaStack* stack, usize size, usize alignment);
#define arena_alloc(arena, elt, ...) \
    raw_arena_stack_alloc((arena), LAYOUT_OF(elt, __VA_ARGS__))

void* raw_arena_stack_extend(
    ArenaStack* stack,
    void* data,
    usize size,
    usize alignment
);
#define arena_stack_push(arena, elt) \
    raw_arena_stack_extend((arena), &(elt), LAYOUT_OF(elt))

typedef struct ArenaStackState {
    DynArena* top;
    void* end;
} ArenaStackState;

ArenaStackState arena_stack_snapshot(ArenaStack stack);
void arena_stack_restore(ArenaStack* stack, ArenaStackState state);
