#pragma once

#include "alloc/array.h"
#include "bytecode/bytecode.h"
#include "compiler/ast/type.h"

typedef ArrayBuf(u64) U64ArrayBuf;

// == structure of the stack ==
// ... [frame n-2] [pos n-2] [frame n-1] [pos n-1] [frame n]
//                                                  ^^^^^^^
//                                                  current frame
//
// == structure of a stack frame ==
// [variables] [expressions]

typedef struct VmPos {
    usize frame_start;
    Byteword* ip;
} VmPos;

typedef struct Primitive {
    union {
        i64 integer;
        void* dynamic;
        usize function;
    } as;
} Primitive;
typedef ArrayBuf(Primitive) ValueStack;

typedef struct Vm {
    Bytecode bytecode;
    ValueStack stack;
    VmPos pos;
    // Gc gc;
    Reporter* reporter;
    i64 exit_code;
} Vm;

// is not responsible for destroying the bytecode
Vm new_vm(Bytecode bytecode, Reporter* reporter);
void free_vm(Vm vm);

int run_vm(Vm* vm);
