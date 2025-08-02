#pragma once

#include "alloc/array.h"
#include "primitives/primitives.h"

typedef void* Pointer;
typedef ArrayBuf(Pointer) PointerArrayBuf;

typedef struct AllocStack {
    PointerArrayBuf allocations;
} AllocStack;

AllocStack new_alloc_stack(void);
void free_alloc_stack(AllocStack registry);
void alloc_stack_push(AllocStack* registry, void* ptr);
usize alloc_stack_snapshot(AllocStack registry);
void alloc_stack_restore(AllocStack* registry, usize state);
