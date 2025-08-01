#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "vm/vm.h"
#include "vm/diagnostics.h"

typedef enum ControlFlow {
    FLOW_EXIT = 0,
    FLOW_CONTINUE = 1,
} ControlFlow;

typedef struct VmFrameCapture {
    const Byteword* ip;
    u64 fp_offset;
} VmFrameCapture;
#define VM_FRAME_CAPTURE_REGISTERS \
    (sizeof(VmFrameCapture) / sizeof(Register))

static VmFrameCapture vm_capture_frame(Vm vm) {
    return (VmFrameCapture){
        .ip = vm.ip,
        .fp_offset = vm.stack.fp - vm.stack.data,
    };
}

static void vm_restore_frame_capture(Vm* vm, VmFrameCapture capture) {
    vm->ip = capture.ip;
    vm->stack.fp = vm->stack.data + capture.fp_offset;
}

VmStack new_vm_stack(void) {
    usize capacity = 8;
    Register* data = malloc(capacity * sizeof(Register));
    return (VmStack){
        .data = data,
        .fp = data,
        .sp = data,
        .ap = data,
    };
}

void free_vm_stack(VmStack stack) {
    free(stack.data);
}

void vm_stack_reserve(VmStack* stack, usize additional_registers) {
    usize size = stack->sp - stack->data;
    usize min_new_capacity = size + additional_registers;
    if (stack->capacity >= min_new_capacity) return;

    usize multiplied_capacity = 1.5 * stack->capacity;
    usize new_capacity = (multiplied_capacity > min_new_capacity) ? multiplied_capacity : min_new_capacity;

    usize fp_offset = stack->fp - stack->data;
    usize sp_offset = stack->sp - stack->data;
    usize ap_offset = stack->ap - stack->data;

    stack->data = realloc(stack->data, new_capacity * sizeof(Register));
    if (stack->data == NULL) {
        print_errno();
        exit(EXIT_FAILURE);
    }

    stack->fp = stack->data + fp_offset;
    stack->sp = stack->data + sp_offset;
    stack->ap = stack->data + ap_offset;
}

Vm new_vm(Bytecode bytecode, Reporter* reporter) {
    return (Vm){
        .reporter = reporter,
        .bytecode = bytecode,
        .ip = bytecode.instructions.data,
        .stack = new_vm_stack(),
        .exit_code = 0,
    };
}

void free_vm(Vm vm) {
    free_array_buf(vm.stack);
}

static ControlFlow run_one(Vm* vm);

static ControlFlow op_sys(Vm* vm);

static ControlFlow op_frm(Vm* vm);
static ControlFlow op_arg(Vm* vm);
static ControlFlow op_cas(Vm* vm);
static ControlFlow op_res(Vm* vm);
static ControlFlow op_ret(Vm* vm);

static ControlFlow op_sca(Vm* vm);
static ControlFlow op_loa(Vm* vm);
static ControlFlow op_sto(Vm* vm);
static ControlFlow op_mov(Vm* vm);

static ControlFlow op_adu(Vm* vm);

static ControlFlow sys_exit(Vm* vm);
static ControlFlow sys_hi(Vm* vm);
static ControlFlow sys_bye(Vm* vm);
static ControlFlow sys_dbg(Vm* vm);

void run_vm(Vm* vm) {
    while (true){
        if (run_one(vm) == FLOW_EXIT) {
            return;
        }
    }
}

static ControlFlow run_one(Vm* vm) {
    switch ((Opcode)(*(vm->ip++))) {
    case OP_NOP: return FLOW_CONTINUE;
    
    case OP_SYS: return op_sys(vm);
    
    case OP_FRM: return op_frm(vm);
    case OP_ARG: return op_arg(vm);
    case OP_CAS: return op_cas(vm);
    case OP_RES: return op_res(vm);
    case OP_RET: return op_ret(vm);

    case OP_SCA: return op_sca(vm);
    case OP_LOA: return op_loa(vm);
    case OP_STO: return op_sto(vm);
    case OP_MOV: return op_mov(vm);

    case OP_ADU: return op_adu(vm);

    // FIXME: invalid opcode
    }
}

static const Byteword* align_up(const Byteword* ptr, usize alignment) {
    uptr addr = (uptr)ptr;
    uptr mask = -alignment;
    addr = (addr + ~mask) & mask;
    return (const Byteword*)addr;
}

static ControlFlow op_sys(Vm* vm) {
    switch ((Syscall)(*(vm->ip++))) {
    case SYS_NOP: return FLOW_CONTINUE;

    case SYS_EXIT: return sys_exit(vm);
    
    case SYS_HI: return sys_hi(vm);
    case SYS_BYE: return sys_bye(vm);

    case SYS_DBG: return sys_dbg(vm);

    // FIXME: invalid syscall
    }
}

static ControlFlow op_frm(Vm* vm) {
    usize nargs = *(vm->ip++);
    vm_stack_reserve(&vm->stack, VM_FRAME_CAPTURE_REGISTERS + nargs);

    VmFrameCapture capture = vm_capture_frame(*vm);
    *((VmFrameCapture*)vm->stack.sp) = capture;
    vm->stack.ap = vm->stack.sp + VM_FRAME_CAPTURE_REGISTERS;

    return FLOW_CONTINUE;
}

static ControlFlow op_arg(Vm* vm) {
    Byteword reg = *(vm->ip++);
    *(vm->stack.ap++) = *(vm->stack.fp + reg);
    return FLOW_CONTINUE;
}

static ControlFlow op_cas(Vm* vm) {
    // FIXME: portable alignment
    const u64* ip = (const u64*)align_up(vm->ip, alignof(u64));
    u64 func = *(ip++);

    VmFrameCapture* capture = (VmFrameCapture*)vm->stack.sp;
    capture->ip = (const Byteword*)ip;

    vm->ip = vm->bytecode.instructions.data + func;
    vm->stack.fp = vm->stack.sp + VM_FRAME_CAPTURE_REGISTERS;
    vm->stack.sp = vm->stack.ap;
    return FLOW_CONTINUE;
}

static ControlFlow op_res(Vm* vm) {
    Byteword nregs = *(vm->ip++);
    vm_stack_reserve(&vm->stack, nregs);
    vm->stack.sp += nregs;
    return FLOW_CONTINUE;
}

static ControlFlow op_ret(Vm* vm) {
    Byteword value_start = *(vm->ip++);
    Byteword value_nregs = *(vm->ip++);
    Register* value = vm->stack.fp + value_start;

    VmFrameCapture* capture = (VmFrameCapture*)(vm->stack.fp - VM_FRAME_CAPTURE_REGISTERS);
    vm_restore_frame_capture(vm, *capture);
    vm->stack.sp = (Register*)capture;

    // correct as sp < value: value[..] won't be overwritten.
    for (size_t i = 0; i < value_nregs; i++) {
        vm->stack.sp[i] = value[i];
    }

    return FLOW_CONTINUE;
}

static ControlFlow op_sca(Vm* vm) {
    Byteword reg_idx = (*vm->ip++);
    const Register* valp = (const Register*)align_up(vm->ip, alignof(u64));
    Register val = *(valp++);
    vm->ip = (const Byteword*)valp;
    vm->stack.fp[reg_idx] = val;
    return FLOW_CONTINUE;
}

static ControlFlow op_loa(Vm* vm) {
    Byteword dst = *(vm->ip++);
    Byteword src_ptr_idx = *(vm->ip++);
    const Register* src_ptr = (const Register*)vm->stack.fp[src_ptr_idx];
    vm->stack.fp[dst] = *src_ptr;
    return FLOW_CONTINUE;
}

static ControlFlow op_sto(Vm* vm) {
    Byteword dst_ptr_idx = *(vm->ip++);
    Byteword src = *(vm->ip++);
    Register* dst_ptr = (Register*)vm->stack.fp[dst_ptr_idx];
    *dst_ptr = vm->stack.fp[src];
    return FLOW_CONTINUE;
}

static ControlFlow op_mov(Vm* vm) {
    Byteword dst = *(vm->ip++);
    Byteword src = *(vm->ip++);
    vm->stack.fp[dst] = vm->stack.fp[src];
    return FLOW_CONTINUE;
}

static ControlFlow op_adu(Vm* vm) {
    Byteword dst = *(vm->ip++);
    Byteword op1 = *(vm->ip++);
    Byteword op2 = *(vm->ip++);
    vm->stack.fp[dst] = (u64)(vm->stack.fp[op1]) + (u64)(vm->stack.fp[op2]);
    return FLOW_CONTINUE;
}

static ControlFlow sys_exit(Vm* vm) {
    Byteword exit_code = *(vm->ip++);
    vm->exit_code = vm->stack.fp[exit_code];
    return FLOW_EXIT;
}

static ControlFlow sys_hi(Vm* vm) {
    eprintf("[sys hi]   *coughs* hi!!\n");
    return FLOW_CONTINUE;
}

static ControlFlow sys_bye(Vm* vm) {
    eprintf("[sys bye]  *coughs* bye!!\n");
    return FLOW_CONTINUE;
}

static ControlFlow sys_dbg(Vm* vm) {
    Byteword reg = *(vm->ip++);
    u64 val = vm->stack.fp[reg];
    eprintf("[sys dbg]  %d: %" PRIu64 " %" PRIx64 "\n", (int)reg, val, val);
    return FLOW_CONTINUE;
}
