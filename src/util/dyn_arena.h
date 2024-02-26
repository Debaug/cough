#pragma once

#include <stdlib.h>
#include <stdalign.h>

#include "util/alloc.h"

typedef struct dyn_arena {
    void* data;
    size_t end;
    size_t capacity;
} dyn_arena_t;

dyn_arena_t new_dyn_arena(void);
void free_dyn_arena(dyn_arena_t arena);

handle_t dyn_arena_alloc(dyn_arena_t* arena, size_t size, size_t alignment);
#define dyn_arena_alloc_for(arena, elt) \
    dyn_arena_alloc((arena), LAYOUT_OF(elt))
#define dyn_arena_alloc_for_array(arena, elt, n)    \
    dyn_arena_alloc((arena), LAYOUT_OF(elt, n))

handle_t dyn_arena_extend(
    dyn_arena_t* dyn_arena,
    void* data,
    size_t size, 
    size_t alignment
);
#define dyn_arena_push(arena, elt)  \
    dyn_arena_extend((arena), &(elt), LAYOUT_OF(elt))

#define dyn_arena_resolve(arena, handle, T) ((T*)((arena).data + (handle)))
#define dyn_arena_get(arena, handle, T) *dyn_arena_resolve(arena, handle, T)

typedef size_t dyn_arena_state_t;

dyn_arena_state_t dyn_arena_snapshot(dyn_arena_t arena);
void dyn_arena_restore(dyn_arena_t* arena, dyn_arena_state_t state);
