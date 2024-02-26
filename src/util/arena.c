#include "util/arena.h"
#include "util/array.h"

arena_t new_arena(void) {
    return (arena_t){ .data = NULL, .end = 0, .capacity = 0 };
}

void free_arena(arena_t arena) {
    free(arena.data);
}

handle_t arena_alloc(arena_t* arena, size_t size, size_t alignment) {
    handle_t handle = arena->end = __builtin_align_up(arena->end, alignment);
    raw_array_buf_reserve(&arena->data, handle, &arena->capacity, size, 1);
    arena->end += size;
    return handle;
}

handle_t arena_extend(arena_t* arena, void* data, size_t size, size_t alignment) {
    handle_t handle = arena->end = __builtin_align_up(arena->end, alignment);
    raw_array_buf_extend(&arena->data, &arena->end, &arena->capacity, data, size, 1);
    return handle;
}

size_t arena_snapshot(arena_t arena) {
    return arena.end;
}

void arena_restore(arena_t* arena, size_t state) {
    arena->end = state;
}
