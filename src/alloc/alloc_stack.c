#include "alloc/alloc_stack.h"

alloc_stack_t new_alloc_stack(void) {
    return (alloc_stack_t){ .allocations = new_array_buf(pointer_t) };
}

void free_alloc_stack(alloc_stack_t stack) {
    free_array_buf(stack.allocations);
}

void alloc_stack_push(alloc_stack_t* stack, void* ptr) {
    array_buf_push(&stack->allocations, ptr);
}

size_t alloc_stack_snapshot(alloc_stack_t stack) {
    return stack.allocations.len;
}

void alloc_stack_restore(alloc_stack_t* stack, size_t state) {
    for (size_t i = state; i < stack->allocations.len; i++) {
        free(stack->allocations.data[i]);
    }
    stack->allocations.len = state;
}
