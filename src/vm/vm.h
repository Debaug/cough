#pragma once

#include "alloc/array.h"
#include "bytecode/bytecode.h"
#include "ast/type.h"

typedef array_buf_t(uint64_t) uint64_array_buf_t;

// == structure of the stack ==
// ... [frame n-2] [pos n-2] [frame n-1] [pos n-1] [frame n]
//                                                  ^^^^^^^
//                                                  current frame
//
// == structure of a stack frame ==
// [variables] [expressions]

typedef struct vm_pos {
    size_t frame_start;
    byteword_t* ip;
} vm_pos_t;

typedef struct primitive {
    union {
        int64_t integer;
        void* dynamic;
        size_t function;
    } as;
} primitive_t;
typedef array_buf_t(primitive_t) value_stack_t;

typedef struct vm {
    bytecode_t bytecode;
    value_stack_t stack;
    vm_pos_t pos;
    // gc_t gc;
    reporter_t* reporter;
    int64_t exit_code;
} vm_t;

// is not responsible for destroying the bytecode
vm_t new_vm(bytecode_t bytecode, reporter_t* reporter);
void free_vm(vm_t vm);

int run_vm(vm_t* vm);
