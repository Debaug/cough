#pragma once

#include "alloc/array.h"
#include "compiler/compiler.h"
#include "ast/type.h"

typedef enum marker {
    MARKER_A = 0,
    MARKER_B = 1,
} marker_t;

marker_t flip_marker(marker_t marker);

typedef struct allocation {
    void* pointer; // null if freed
    size_t* subobject_offsets;
    size_t subobject_offsets_len;
    marker_t marker;
} allocation_t;

typedef struct gc {
    // array_buf_t /* allocation_t */ allocations;
    // array_buf_t /* size_t */ freed;
    // array_buf_t /* size_t */ roots;
    marker_t garbage_marker;
} gc_t;

gc_t new_gc();

typedef array_buf_t(uint64_t) uint64_array_buf_t;

typedef struct vm {
    bytecode_t bytecode;
    uint64_array_buf_t variable_stack;
    size_t variable_frame_index;
    uint64_array_buf_t expression_stack;
    uint32_t* ip;
    gc_t gc;
} vm_t;

vm_t new_vm(bytecode_t bytecode);
void run_vm(vm_t vm);
