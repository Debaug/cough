#pragma once

#include "alloc/array.h"
#include "bytecode/bytecode.h"
#include "compiler/ast/type.h"
#include "vm/system.h"

typedef struct VmStack {
    Word* data;
    Word* fp;
    Word* sp;
    Word* ap;
    usize capacity;
} VmStack;

VmStack new_vm_stack(void);
void free_vm_stack(VmStack stack);

/// @brief
/// @param stack the stack.
/// @param additional additional registers (counted from `sp`).
void vm_stack_reserve(VmStack* stack, usize additional_registers);

typedef struct VmComparison {
    bool eq;
    bool gt;
} VmComparison;

typedef struct Vm {
    VmSystem* system;
    Reporter* reporter;
    Bytecode bytecode;
    const Byteword* ip;
    VmStack stack;
    VmComparison comparison;
} Vm;

// the VM is not responsible for destroying the bytecode
Vm new_vm(VmSystem* system, Bytecode bytecode, Reporter* reporter);
void free_vm(Vm vm);

void run_vm(Vm* vm);
