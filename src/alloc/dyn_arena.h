#pragma once

#include <stdlib.h>
#include <stdalign.h>

#include "primitives/primitives.h"
#include "alloc/alloc.h"

typedef struct DynArena {
    void* data;
    usize end;
    usize capacity;
} DynArena;

DynArena new_dyn_arena(void);
void free_dyn_arena(DynArena arena);

Handle dyn_arena_alloc(DynArena* arena, usize size, usize alignment);
#define dyn_arena_alloc_for(arena, elt) \
    dyn_arena_alloc((arena), LAYOUT_OF(elt))
#define dyn_arena_alloc_for_array(arena, elt, n)    \
    dyn_arena_alloc((arena), LAYOUT_OF(elt, n))

Handle dyn_arena_extend(
    DynArena* DynArena,
    void* data,
    usize size, 
    usize alignment
);
#define dyn_arena_push(arena, elt)  \
    dyn_arena_extend((arena), &(elt), LAYOUT_OF(elt))

#define dyn_arena_resolve(arena, handle, T) ((T*)((arena).data + (handle)))
#define dyn_arena_get(arena, handle, T) *dyn_arena_resolve(arena, handle, T)

typedef usize DynArenaState;

DynArenaState dyn_arena_snapshot(DynArena arena);
void dyn_arena_restore(DynArena* arena, DynArenaState state);
