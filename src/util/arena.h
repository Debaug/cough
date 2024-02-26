#pragma once

#include <stdlib.h>
#include <stdalign.h>

typedef size_t handle_t;
#define HANDLE_NULL ((handle_t)(-1))

typedef struct arena {
    void* data;
    size_t end;
    size_t capacity;
} arena_t;

arena_t new_arena(void);
void free_arena(arena_t arena);

#define LAYOUT_OF(elt) sizeof(elt), alignof(elt)
#define LAYOUT_OF_ARRAY(elt, n) sizeof(elt) * (n), alignof(elt)

handle_t arena_alloc(arena_t* arena, size_t size, size_t alignment);
#define arena_alloc_for(arena, elt) arena_alloc((arena), LAYOUT_OF(elt))
#define arena_alloc_for_array(arena, elt, n)    \
    arena_alloc((arena), LAYOUT_OF_ARRAY(elt, n))

handle_t arena_extend(arena_t* arena, void* data, size_t size, size_t alignment);
#define arena_push(arena, elt) arena_extend((arena), &(elt), LAYOUT_OF(elt))

#define arena_resolve(arena, handle, T) ((T*)((arena).data + (handle)))
#define arena_get(arena, handle, T) *arena_resolve(arena, handle, T)

size_t arena_snapshot(arena_t arena);
void arena_restore(arena_t* arena, size_t state);
