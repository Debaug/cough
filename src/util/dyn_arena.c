#include "util/dyn_arena.h"
#include "util/array.h"

dyn_arena_t new_dyn_arena(void) {
    return (dyn_arena_t){ .data = NULL, .end = 0, .capacity = 0 };
}

void free_dyn_arena(dyn_arena_t arena) {
    free(arena.data);
}

handle_t dyn_arena_alloc(dyn_arena_t* arena, size_t size, size_t alignment) {
    handle_t handle = arena->end = __builtin_align_up(arena->end, alignment);
    raw_array_buf_reserve(&arena->data, handle, &arena->capacity, size, 1);
    arena->end += size;
    return handle;
}

handle_t dyn_arena_extend(
    dyn_arena_t* arena,
    void* data,
    size_t size, 
    size_t alignment
) {
    handle_t handle = arena->end = __builtin_align_up(arena->end, alignment);
    raw_array_buf_extend(&arena->data, &arena->end, &arena->capacity, data, size, 1);
    return handle;
}

size_t arena_snapshot(dyn_arena_t arena) {
    return arena.end;
}

void arena_restore(dyn_arena_t* arena, size_t state) {
    arena->end = state;
}
