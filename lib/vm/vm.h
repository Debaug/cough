#pragma once

#include "bytecode/bytecode.h"
#include "vm/system.h"
#include "diagnostics/report.h"

typedef struct VmStack {
    Word* data;
    Word* fp;
    Word* sp;
    Word* ap;
    usize capacity;
} VmStack;

VmStack vm_stack_new(void);
void vm_stack_free(VmStack* stack);

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
Vm vm_new(VmSystem* system, Bytecode bytecode, Reporter* reporter);
void vm_free(Vm* vm);

void vm_run(Vm* vm);
