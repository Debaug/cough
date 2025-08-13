#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "vm/vm.h"
#include "vm/diagnostics.h"

VmStack new_vm_stack(void) {
    usize capacity = 8;
    Word* data = malloc(capacity * sizeof(Word));
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

    stack->data = realloc(stack->data, new_capacity * sizeof(Word));
    if (stack->data == NULL) {
        print_errno();
        exit(EXIT_FAILURE);
    }

    stack->fp = stack->data + fp_offset;
    stack->sp = stack->data + sp_offset;
    stack->ap = stack->data + ap_offset;
}

Vm new_vm(VmSystem* system, Bytecode bytecode, Reporter* reporter) {
    return (Vm){
        .system = system,
        .reporter = reporter,
        .bytecode = bytecode,
        .ip = bytecode.instructions.data,
        .stack = new_vm_stack(),
    };
}

void free_vm(Vm vm) {
    free_array_buf(vm.stack);
}

typedef enum ControlFlow {
    FLOW_EXIT = 0,
    FLOW_CONTINUE = 1,
} ControlFlow;

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

static ControlFlow op_jmp(Vm* vm);
static ControlFlow op_jnz(Vm* vm);

static ControlFlow op_equ(Vm* vm);
static ControlFlow op_neu(Vm* vm);
static ControlFlow op_geu(Vm* vm);
static ControlFlow op_gtu(Vm* vm);

static ControlFlow op_adu(Vm* vm);

static ControlFlow sys_nop(Vm* vm);
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

typedef struct VmFrameCapture {
    const Byteword* ip;
    u64 fp_offset;
} VmFrameCapture;
#define VM_FRAME_CAPTURE_REGISTERS \
    (sizeof(VmFrameCapture) / sizeof(Word))

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

static Word* reg_ptr(Vm* vm, usize idx) {
    return vm->stack.fp + idx;
}

static Word reg_val(const Vm* vm, usize idx) {
    return vm->stack.fp[idx];
}

static Byteword fetch_imm_byteword(Vm* vm) {
    return *(vm->ip++);
}

static Word fetch_imm_word(Vm* vm) {
    const Word* ptr = (const Word*)align_up(vm->ip, alignof(Word));
    Word imm = *(ptr++);
    vm->ip = (const Byteword*)ptr;
    return imm;
}

static usize fetch_reg(Vm* vm) {
    return fetch_imm_byteword(vm);
}

static Word* fetch_reg_ptr(Vm* vm) {
    return reg_ptr(vm, fetch_reg(vm));
}

static Word fetch_reg_val(Vm* vm) {
    return *fetch_reg_ptr(vm);
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

    case OP_JMP: return op_jmp(vm);
    case OP_JNZ: return op_jnz(vm);

    case OP_EQU: return op_equ(vm);
    case OP_NEU: return op_neu(vm);
    case OP_GEU: return op_geu(vm);
    case OP_GTU: return op_gtu(vm);

    case OP_ADU: return op_adu(vm);

    // FIXME: invalid opcode
    }
}

static ControlFlow op_sys(Vm* vm) {
    switch ((Syscall)(*(vm->ip++))) {
    case SYS_NOP: return sys_nop(vm);

    case SYS_EXIT: return sys_exit(vm);
    
    case SYS_HI: return sys_hi(vm);
    case SYS_BYE: return sys_bye(vm);

    case SYS_DBG: return sys_dbg(vm);

    // FIXME: invalid syscall
    }
}

static ControlFlow op_frm(Vm* vm) {
    usize nargs = fetch_imm_byteword(vm);
    vm_stack_reserve(&vm->stack, VM_FRAME_CAPTURE_REGISTERS + nargs);

    VmFrameCapture capture = vm_capture_frame(*vm);
    *((VmFrameCapture*)vm->stack.sp) = capture;
    vm->stack.ap = vm->stack.sp + VM_FRAME_CAPTURE_REGISTERS;

    return FLOW_CONTINUE;
}

static ControlFlow op_arg(Vm* vm) {
    *(vm->stack.ap++) = fetch_reg_val(vm);
    return FLOW_CONTINUE;
}

static ControlFlow op_cas(Vm* vm) {
    usize func = fetch_imm_word(vm).as_uint;

    VmFrameCapture* capture = (VmFrameCapture*)vm->stack.sp;
    capture->ip = vm->ip;

    vm->ip = vm->bytecode.instructions.data + func;
    vm->stack.fp = vm->stack.sp + VM_FRAME_CAPTURE_REGISTERS;
    vm->stack.sp = vm->stack.ap;
    return FLOW_CONTINUE;
}

static ControlFlow op_res(Vm* vm) {
    usize nregs = fetch_imm_byteword(vm);
    vm_stack_reserve(&vm->stack, nregs);
    vm->stack.sp += nregs;
    return FLOW_CONTINUE;
}

static ControlFlow op_ret(Vm* vm) {
    Word* value = fetch_reg_ptr(vm);
    usize len = fetch_imm_byteword(vm);

    VmFrameCapture* capture = (VmFrameCapture*)(vm->stack.fp - VM_FRAME_CAPTURE_REGISTERS);
    vm_restore_frame_capture(vm, *capture);
    vm->stack.sp = (Word*)capture;

    // correct as sp < value: value[..] won't be overwritten.
    for (size_t i = 0; i < len; i++) {
        vm->stack.sp[i] = value[i];
    }

    return FLOW_CONTINUE;
}

static ControlFlow op_sca(Vm* vm) {
    Word* reg = fetch_reg_ptr(vm);
    Word val = fetch_imm_word(vm);
    *reg = val;
    return FLOW_CONTINUE;
}

static ControlFlow op_loa(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    const Word* src = (const Word*)(fetch_reg_val(vm).as_ptr);
    return FLOW_CONTINUE;
}

static ControlFlow op_sto(Vm* vm) {
    Word* dst = (Word*)(fetch_reg_val(vm).as_mut_ptr);
    Word src = fetch_reg_val(vm);
    *dst = src;
    return FLOW_CONTINUE;
}

static ControlFlow op_mov(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    Word src = fetch_reg_val(vm);
    *dst = src;
    return FLOW_CONTINUE;
}

static ControlFlow op_jmp(Vm* vm) {
    usize dst = fetch_imm_word(vm).as_uint;
    vm->ip = vm->bytecode.instructions.data + dst;
    return FLOW_CONTINUE;
}

static ControlFlow op_jnz(Vm* vm) {
    usize dst = fetch_imm_word(vm).as_uint;
    Word val = fetch_reg_val(vm);
    if (val.as_uint != 0) {
        vm->ip = vm->bytecode.instructions.data + dst;
    }
    return FLOW_CONTINUE;
}

static ControlFlow op_equ(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    Word op1 = fetch_reg_val(vm);
    Word op2 = fetch_reg_val(vm);
    dst->as_uint = op1.as_uint == op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_neu(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    Word op1 = fetch_reg_val(vm);
    Word op2 = fetch_reg_val(vm);
    dst->as_uint = op1.as_uint != op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_geu(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    Word op1 = fetch_reg_val(vm);
    Word op2 = fetch_reg_val(vm);
    dst->as_uint = op1.as_uint >= op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_gtu(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    Word op1 = fetch_reg_val(vm);
    Word op2 = fetch_reg_val(vm);
    dst->as_uint = op1.as_uint > op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_adu(Vm* vm) {
    Word* dst = fetch_reg_ptr(vm);
    Word op1 = fetch_reg_val(vm);
    Word op2 = fetch_reg_val(vm);
    dst->as_uint = op1.as_uint + op2.as_uint;
    return FLOW_CONTINUE;
}
static ControlFlow sys_nop(Vm* vm) {
    (vm->system)->vtable->nop(vm->system);
    return FLOW_CONTINUE;
}

static ControlFlow sys_exit(Vm* vm) {
    i64 exit_code = fetch_reg_val(vm).as_int;
    (vm->system)->vtable->exit(vm->system, exit_code);
    return FLOW_EXIT;
}

static ControlFlow sys_hi(Vm* vm) {
    (vm->system)->vtable->hi(vm->system);
    return FLOW_CONTINUE;
}

static ControlFlow sys_bye(Vm* vm) {
    (vm->system)->vtable->bye(vm->system);
    return FLOW_CONTINUE;
}

static ControlFlow sys_dbg(Vm* vm) {
    usize idx = fetch_reg(vm);
    Word val = reg_val(vm, idx);
    (vm->system)->vtable->dbg(vm->system, idx, val);
    return FLOW_CONTINUE;
}
