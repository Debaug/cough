#pragma once

#include <stdlib.h>
#include <stdalign.h>

#include "alloc/buf.h"

typedef struct ArenaStack {
    Buf _top;
} ArenaStack;

ArenaStack new_arena_stack(void);
void free_arena_stack(ArenaStack* stack);

// alignment must be less (less strict) than `alignof(max_align_t)`
void* arena_stack_alloc(ArenaStack* stack, usize size, usize alignment);
void* arena_stack_extend(ArenaStack* stack, void const* src, usize size, usize alignment);
