#pragma once

#include "util/array.h"

typedef void* pointer_t;
typedef array_buf_t(pointer_t) pointer_array_buf_t;

typedef struct alloc_stack {
    pointer_array_buf_t allocations;
} alloc_stack_t;

alloc_stack_t new_alloc_stack(void);
void free_alloc_stack(alloc_stack_t registry);
void alloc_stack_push(alloc_stack_t* registry, void* ptr);
size_t alloc_stack_snapshot(alloc_stack_t registry);
void alloc_stack_restore(alloc_stack_t* registry, size_t state);
