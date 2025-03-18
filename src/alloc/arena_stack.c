#include <string.h>

#include "alloc/arena_stack.h"
#include "diagnostic/diagnostic.h"

// should fit in a page
#define ARENA_SIZE 2048
#define ARENA_ALIGNMENT 2048

typedef struct DynArena {
    DynArena* previous;
    void* end;
    // data...
} DynArena;

ArenaStack new_arena_stack(void) {
    return (ArenaStack){
        .top = NULL
    };
}

void free_arena_stack(ArenaStack stack) {
    while (stack.top != NULL) {
        DynArena* arena_ptr = stack.top;
        stack.top = arena_ptr->previous;
        free(arena_ptr);
    }
}

static void push_arena(ArenaStack* stack) {
    DynArena* arena = aligned_alloc(ARENA_ALIGNMENT, ARENA_SIZE);
    arena->previous = stack->top;
    arena->end = arena + sizeof(DynArena);
    stack->top = arena;
}

void* raw_arena_stack_alloc(ArenaStack* stack, usize size, usize alignment) {
    if (size + sizeof(DynArena) > ARENA_SIZE) {
        print_error("tried to allocated too large of a memory block for a single arena (%zu bytes)", size);
    }

    if (stack->top == NULL) {
        push_arena(stack);
    }

    void* ptr = __builtin_align_up(stack->top->end, alignment);
    if (ptr + size > (void*)stack->top + ARENA_SIZE) {
        push_arena(stack);
        ptr = __builtin_align_up(stack->top->end, alignment);
    }

    stack->top->end = ptr + size;
    return ptr;
}

void* raw_arena_stack_extend(
    ArenaStack* stack,
    void* data,
    usize size,
    usize alignment
) {
    void* ptr = raw_arena_stack_alloc(stack, size, alignment);
    memcpy(ptr, data, size);
    return ptr;
}

ArenaStackState arena_stack_snapshot(ArenaStack stack) {
    if (stack.top == NULL) {
        return (ArenaStackState){ .top = stack.top, .end = 0 };
    }
    return (ArenaStackState){ .top = stack.top, .end = stack.top->end };
}

void arena_stack_restore(ArenaStack* stack, ArenaStackState state) {
    while (stack->top != state.top) {
        DynArena* top_arena = stack->top;
        stack->top = top_arena->previous;
        free(top_arena);
    }
    if (stack->top) {
        stack->top->end = state.end;
    }
}
