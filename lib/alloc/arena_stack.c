#include <string.h>

#include "alloc/arena_stack.h"
#include "diagnostics/diagnostics.h"
#include "ops/ptr.h"

ArenaStack new_arena_stack(void) {
    return (ArenaStack){
        ._top = buf_new(0),
    };
}

void free_arena_stack(ArenaStack* stack) {
    Buf node = stack->_top;
    while (node.data != NULL) {
        Buf next_node = *(const Buf*)node.data;
        buf_free(&node);
        node = next_node;
    }
}

void* arena_stack_alloc(ArenaStack* stack, usize size, usize alignment) {
    void* ptr = buf_alloc(&stack->_top, size, alignment);
    if (ptr) {
        return ptr;
    }

    usize next_capacity = (stack->_top.capacity <= 32) ? 64 : stack->_top.capacity * 2;
    next_capacity = (next_capacity >= 2 * sizeof(Buf)) ? next_capacity : sizeof(Buf);
    next_capacity = (next_capacity >= size) ? next_capacity : size;
    next_capacity = align_up_size(next_capacity, alignof(max_align_t));
    Buf next_buf = buf_new(next_capacity);
    buf_extend(&next_buf, &stack->_top, sizeof(stack->_top), 1);
    stack->_top = next_buf;
    return buf_alloc(&stack->_top, size, alignment);
}

void* arena_stack_extend(ArenaStack* stack, void const* src, usize size, usize alignment) {
    void* dst = arena_stack_alloc(stack, size, alignment);
    memcpy(dst, src, size);
    return dst;
}
