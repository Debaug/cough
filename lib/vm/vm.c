#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "alloc/alloc.h"
#include "vm/vm.h"
#include "vm/diagnostics.h"

VmStack vm_stack_new(void) {
    Word* data = malloc_or_exit(8 * sizeof(Word));
    return (VmStack){
        .data = data,
        .fp = data,
        .sp = data,
        .ap = data,
        .capacity = 8,
    };
}

void vm_stack_free(VmStack* stack) {
    free(stack->data);
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

    stack->data = realloc_or_exit(stack->data, new_capacity * sizeof(Word));

    stack->fp = stack->data + fp_offset;
    stack->sp = stack->data + sp_offset;
    stack->ap = stack->data + ap_offset;
}

Vm vm_new(VmSystem* system, Bytecode bytecode, Reporter* reporter) {
    return (Vm){
        .system = system,
        .reporter = reporter,
        .bytecode = bytecode,
        .ip = bytecode.instructions.data,
        .stack = vm_stack_new(),
    };
}

void vm_free(Vm* vm) {
    vm_stack_free(&vm->stack);
}

typedef enum ControlFlow {
    FLOW_EXIT = 0,
    FLOW_CONTINUE = 1,
} ControlFlow;

static ControlFlow run_one(Vm* vm);

#define Arg(mnemo) Arg_##mnemo
typedef Byteword    Arg(imb);
typedef Word        Arg(imw);
typedef Word*       Arg(preg);
typedef Word        Arg(reg);
typedef usize       Arg(loc);

#define DECL_ARG(idx, mnemo, ...) , Arg(mnemo) x##idx

#define DECL_OP_FN(code, mnemo, ...)                            \
    static ControlFlow op_##mnemo(                              \
        Vm* vm                                                  \
        FOR_ALL(DECL_ARG __VA_OPT__(, __VA_ARGS__))             \
    );
FOR_OPERATIONS(DECL_OP_FN)

#define DECL_SYS_FN(code, mnemo, ...)                           \
    static ControlFlow sys_##mnemo(                             \
        Vm* vm                                                  \
        FOR_ALL(DECL_ARG __VA_OPT__(, __VA_ARGS__))             \
    );
FOR_SYSCALLS(DECL_SYS_FN)

void vm_run(Vm* vm) {
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
#define VM_FRAME_CAPTURE_REGISTERS (sizeof(VmFrameCapture) / sizeof(Word))

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

static Opcode fetch_op(Vm* vm) {
    return bytecode_read_opcode(&vm->ip);
}

static Syscall fetch_sys(Vm* vm) {
    return bytecode_read_syscall(&vm->ip);
}

static Byteword fetch_imb(Vm* vm) {
    return bytecode_read_byteword(&vm->ip);
}

static Word fetch_imw(Vm* vm) {
    return bytecode_read_word(&vm->ip);
}

static Word* fetch_preg(Vm* vm) {
    return vm->stack.fp + fetch_imb(vm);
}

static Word fetch_reg(Vm* vm) {
    return *fetch_preg(vm);
}

static usize fetch_loc(Vm* vm) {
    return bytecode_read_location(&vm->ip);
}

static ControlFlow run_one(Vm* vm) {
    Opcode opcode = fetch_op(vm);
    #define FETCH(idx, kind, ...) , fetch_##kind(vm)
    switch (opcode) {
    #define RUN_ONE_CASE(code, mnemo, ...)                  \
        case code:                                          \
            return op_##mnemo(                              \
                vm                                          \
                FOR_ALL(FETCH __VA_OPT__(, __VA_ARGS__))    \
            );
    FOR_OPERATIONS(RUN_ONE_CASE)

    case OP_SYS:;
        Syscall syscall = fetch_sys(vm);
        switch (syscall) {
        #define RUN_ONE_SYS_CASE(code, mnemo, ...)              \
            case code:                                          \
                return sys_##mnemo(                             \
                    vm                                          \
                    FOR_ALL(FETCH __VA_OPT__(, __VA_ARGS__))    \
                );
        FOR_SYSCALLS(RUN_ONE_SYS_CASE)
        default:
            // FIXME: invalid opcode
            exit(-1);
            return FLOW_EXIT;
        }

    default:
        // FIXME: invalid opcode
        exit(-1);
        return FLOW_EXIT;
    }
}

static ControlFlow op_nop(Vm* vm) {
    return FLOW_CONTINUE;
}

static ControlFlow op_frm(Vm* vm, Byteword nargs) {
    vm_stack_reserve(&vm->stack, VM_FRAME_CAPTURE_REGISTERS + nargs);

    VmFrameCapture capture = vm_capture_frame(*vm);
    *((VmFrameCapture*)vm->stack.sp) = capture;
    vm->stack.ap = vm->stack.sp + VM_FRAME_CAPTURE_REGISTERS;

    return FLOW_CONTINUE;
}

static ControlFlow op_arg(Vm* vm, Word val) {
    *(vm->stack.ap++) = val;
    return FLOW_CONTINUE;
}

static ControlFlow op_cas(Vm* vm, usize func) {
    VmFrameCapture* capture = (VmFrameCapture*)vm->stack.sp;
    capture->ip = vm->ip;

    vm->ip = vm->bytecode.instructions.data + func;
    vm->stack.fp = vm->stack.sp + VM_FRAME_CAPTURE_REGISTERS;
    vm->stack.sp = vm->stack.ap;
    return FLOW_CONTINUE;
}

static ControlFlow op_res(Vm* vm, Byteword nregs) {
    vm_stack_reserve(&vm->stack, nregs);
    vm->stack.sp += nregs;
    return FLOW_CONTINUE;
}

static ControlFlow op_ret(Vm* vm, Word* start, Byteword len) {
    VmFrameCapture* capture = (VmFrameCapture*)(vm->stack.fp - VM_FRAME_CAPTURE_REGISTERS);
    vm_restore_frame_capture(vm, *capture);
    vm->stack.sp = (Word*)capture;

    // correct as sp < value: value[..] won't be overwritten.
    for (size_t i = 0; i < len; i++) {
        vm->stack.sp[i] = start[i];
    }

    return FLOW_CONTINUE;
}

static ControlFlow op_sca(Vm* vm, Word* dst, Word val) {
    eprintf("dbg: op_sca with val: %d\n", (int)val.as_int);
    *dst = val;
    return FLOW_CONTINUE;
}

static ControlFlow op_loc(Vm* vm, Word* dst, usize val) {
    dst->as_uint = val;
    return FLOW_CONTINUE;
}

static ControlFlow op_loa(Vm* vm, Word* dst, Word src) {
    *dst = *(const Word*)src.as_ptr;
    return FLOW_CONTINUE;
}

static ControlFlow op_sto(Vm* vm, Word dst, Word src) {
    *(Word*)dst.as_mut_ptr = src;
    return FLOW_CONTINUE;
}

static ControlFlow op_mov(Vm* vm, Word* dst, Word src) {
    *dst = src;
    return FLOW_CONTINUE;
}

static ControlFlow op_jmp(Vm* vm, usize dst) {
    vm->ip = vm->bytecode.instructions.data + dst;
    return FLOW_CONTINUE;
}

static ControlFlow op_jnz(Vm* vm, usize dst, Word test) {
    if (test.as_uint != 0) {
        vm->ip = vm->bytecode.instructions.data + dst;
    }
    return FLOW_CONTINUE;
}

static ControlFlow op_equ(Vm* vm, Word* dst, Word op1, Word op2) {
    dst->as_uint = op1.as_uint == op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_neu(Vm* vm, Word* dst, Word op1, Word op2) {
    dst->as_uint = op1.as_uint != op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_geu(Vm* vm, Word* dst, Word op1, Word op2) {
    dst->as_uint = op1.as_uint >= op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_gtu(Vm* vm, Word* dst, Word op1, Word op2) {
    dst->as_uint = op1.as_uint > op2.as_uint;
    return FLOW_CONTINUE;
}

static ControlFlow op_adu(Vm* vm, Word* dst, Word op1, Word op2) {
    dst->as_uint = op1.as_uint + op2.as_uint;
    return FLOW_CONTINUE;
}
static ControlFlow sys_nop(Vm* vm) {
    (vm->system)->vtable->nop(vm->system);
    return FLOW_CONTINUE;
}

static ControlFlow sys_exit(Vm* vm, Word exit_code) {
    (vm->system)->vtable->exit(vm->system, exit_code.as_int);
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

static ControlFlow sys_dbg(Vm* vm, Word* reg) {
    usize idx = reg - vm->stack.fp;
    (vm->system)->vtable->dbg(vm->system, idx, *reg);
    return FLOW_CONTINUE;
}
