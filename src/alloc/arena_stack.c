#include <string.h>

#include "alloc/arena_stack.h"
#include "diagnostic/diagnostic.h"

// should fit in a page
#define ARENA_SIZE 2048
#define ARENA_ALIGNMENT 2048

typedef struct dyn_arena {
    dyn_arena_t* previous;
    void* end;
    // data...
} dyn_arena_t;

arena_stack_t new_arena_stack(void) {
    return (arena_stack_t){
        .top = NULL
    };
}

void free_arena_stack(arena_stack_t stack) {
    while (stack.top != NULL) {
        dyn_arena_t* arena_ptr = stack.top;
        stack.top = arena_ptr->previous;
        free(arena_ptr);
    }
}

static void push_arena(arena_stack_t* stack) {
    dyn_arena_t* arena = aligned_alloc(ARENA_ALIGNMENT, ARENA_SIZE);
    arena->previous = stack->top;
    arena->end = arena + sizeof(dyn_arena_t);
    stack->top = arena;
}

void* raw_arena_stack_alloc(arena_stack_t* stack, size_t size, size_t alignment) {
    if (size + sizeof(dyn_arena_t) > ARENA_SIZE) {
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
    arena_stack_t* stack,
    void* data,
    size_t size,
    size_t alignment
) {
    void* ptr = raw_arena_stack_alloc(stack, size, alignment);
    memcpy(ptr, data, size);
    return ptr;
}

arena_stack_state_t arena_stack_snapshot(arena_stack_t stack) {
    if (stack.top == NULL) {
        return (arena_stack_state_t){ .top = stack.top, .end = 0 };
    }
    return (arena_stack_state_t){ .top = stack.top, .end = stack.top->end };
}

void arena_stack_restore(arena_stack_t* stack, arena_stack_state_t state) {
    while (stack->top != state.top) {
        dyn_arena_t* top_arena = stack->top;
        stack->top = top_arena->previous;
        free(top_arena);
    }
    if (stack->top) {
        stack->top->end = state.end;
    }
}
