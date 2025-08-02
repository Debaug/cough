#include "alloc/alloc_stack.h"

AllocStack new_alloc_stack(void) {
    return (AllocStack){ .allocations = new_array_buf(Pointer) };
}

void free_alloc_stack(AllocStack stack) {
    free_array_buf(stack.allocations);
}

void alloc_stack_push(AllocStack* stack, void* ptr) {
    array_buf_push(&stack->allocations, ptr);
}

usize alloc_stack_snapshot(AllocStack stack) {
    return stack.allocations.len;
}

void alloc_stack_restore(AllocStack* stack, usize state) {
    for (usize i = state; i < stack->allocations.len; i++) {
        free(stack->allocations.data[i]);
    }
    stack->allocations.len = state;
}
