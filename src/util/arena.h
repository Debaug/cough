#pragma once

#include <stdlib.h>
#include <stdalign.h>

typedef struct arena {
    void* ptr;
    size_t end;
    size_t capacity;
} arena_t;

arena_t new_arena(void);
void free_arena(arena_t arena);

void* arena_alloc(arena_t* arena, size_t size, size_t alignment);
#define arena_alloc_for(arena, elt) \
    arena_alloc(arena, sizeof(elt), alignof(elt))

void* arena_extend(arena_t* arena, void* data, size_t size, size_t alignment);
#define arena_push(arena, elt) \
    arena_extend(arena, &elt, sizeof(elt), alignof(elt))
