#include "alloc/dyn_arena.h"
#include "alloc/array.h"

DynArena new_dyn_arena(void) {
    return (DynArena){ .data = NULL, .end = 0, .capacity = 0 };
}

void free_dyn_arena(DynArena arena) {
    free(arena.data);
}

Handle dyn_arena_alloc(DynArena* arena, usize size, usize alignment) {
    Handle handle = arena->end = __builtin_align_up(arena->end, alignment);
    raw_array_buf_reserve(&arena->data, handle, &arena->capacity, size, 1);
    arena->end += size;
    return handle;
}

Handle dyn_arena_extend(
    DynArena* arena,
    void* data,
    usize size, 
    usize alignment
) {
    Handle handle = arena->end = __builtin_align_up(arena->end, alignment);
    raw_array_buf_extend(&arena->data, &arena->end, &arena->capacity, data, size, 1);
    return handle;
}

usize arena_snapshot(DynArena arena) {
    return arena.end;
}

void arena_restore(DynArena* arena, usize state) {
    arena->end = state;
}
