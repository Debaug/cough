#pragma once

#include "bytecode/bytecode.h"
#include "vm/system.h"
#include "diagnostics/report.h"

typedef struct VmValueStack {
    Word* data;
    Word* top;
    usize capacity; // in words
} VmValueStack;

typedef struct VmFrame {
    Byteword const* return_ip;
    usize return_local_variables; // offset in bytes
    // local variables go here
} VmFrame;

typedef struct VmFrameStack {
    void* data;
    void* top;
    Word* local_variables;  // points to the variable 0 (after the frame metadata)
    usize capacity;         // in bytes
} VmFrameStack;

typedef struct Vm {
    VmSystem* system;
    Reporter* reporter;
    Bytecode bytecode;
    Byteword const* ip;
    VmValueStack value_stack;
    VmFrameStack frames;
} Vm;

// the VM is not responsible for destroying the bytecode
Vm vm_new(VmSystem* system, Bytecode bytecode, Reporter* reporter);
void vm_free(Vm* vm);

void vm_run(Vm* vm);
