#pragma once

#include "alloc/array.h"
#include "bytecode/bytecode.h"
#include "compiler/ast/type.h"

typedef union Word {
    const void* as_ptr;
    void* as_mut_ptr;
    u64 as_uint;
    i64 as_int;
    f64 as_float;
} Word;

typedef struct VmStack {
    Word* data;
    Word* fp;
    Word* sp;
    Word* ap;
    usize capacity;
} VmStack;

VmStack new_vm_stack(void);
void free_vm_stack(VmStack stack);
void vm_stack_reserve(VmStack* stack, usize additional_registers);

/// @brief 
/// @param stack the stack.
/// @param additional additional registers (after sp)
void vm_stack_reserve(VmStack* stack, usize additional);

typedef struct Vm {
    Reporter* reporter;
    Bytecode bytecode;
    const Byteword* ip;
    VmStack stack;
    i64 exit_code;
} Vm;

// the VM is not responsible for destroying the bytecode
Vm new_vm(Bytecode bytecode, Reporter* reporter);
void free_vm(Vm vm);

void run_vm(Vm* vm);
