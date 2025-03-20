#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "vm/vm.h"
#include "diagnostics/diagnostics.h"

typedef enum ControlFlow {
    FLOW_EXIT = 0,
    FLOW_CONTINUE = 1,
} ControlFlow;

Vm new_vm(Bytecode bytecode, Reporter* reporter) {
    return (Vm){
        .bytecode = bytecode,
        .stack = new_array_buf(),
        .pos = {
            .frame_start = 0,
            .ip = bytecode.instructions.data,
        },
        .reporter = reporter,
        .exit_code = 0,
    };
}

void free_vm(Vm vm) {
    free_array_buf(vm.stack);
}

static ControlFlow run_one(Vm* vm);

static ControlFlow run_syscall(Vm* vm);
static ControlFlow run_call(Vm* vm);
static ControlFlow run_enter(Vm* vm);
static ControlFlow run_return(Vm* vm);
static ControlFlow run_load_imm(Vm* vm);

static ControlFlow sys_exit(Vm* vm);
static ControlFlow sys_say_hi(Vm* vm);
static ControlFlow sys_say_bye(Vm* vm);

int run_vm(Vm* vm) {
    while (run_one(vm) == FLOW_CONTINUE) {}
    return vm->exit_code;
}

static Primitive pop(Vm* vm) {
    Primitive val;
    array_buf_pop(&vm->stack, &val);
    return val;
}

static void push(Vm* vm, Primitive value) {
    array_buf_push(&vm->stack, value);
}

static ControlFlow run_one(Vm* vm) {
    Opcode op = (Opcode)(*vm->pos.ip);
    vm->pos.ip++;
    switch (op) {
    case OP_NOP: return FLOW_CONTINUE;
    case OP_SYSCALL: return run_syscall(vm);
    case OP_CALL: return run_call(vm);
    case OP_ENTER: return run_enter(vm);
    case OP_RETURN: return run_return(vm);
    case OP_LOAD_IMM: return run_load_imm(vm);
    default:
        // FIXME: better error reporting
        eprintf("invalid instruction 0x%02x\n", op);
        return FLOW_EXIT;
    }
}

static ControlFlow run_syscall(Vm* vm) {
    Syscall op = (Syscall)(*vm->pos.ip);
    vm->pos.ip++;
    switch (op) {
    case SYS_NOP: return FLOW_CONTINUE;
    case SYS_EXIT: return sys_exit(vm);
    case SYS_SAY_HI: return sys_say_hi(vm);
    case SYS_SAY_BYE: return sys_say_bye(vm);
    default:
        // FIXME: better error reporting
        eprintf("invalid syscall 0x%02x\n", op);
        return FLOW_EXIT;
    }
}

static ControlFlow run_call(Vm* vm) {
    // FIXME
    assert(sizeof(VmPos) % sizeof(Primitive) == 0);

    array_buf_extend(
        &vm->stack,
        &vm->pos,
        sizeof(VmPos) / sizeof(Primitive),
        Primitive
    );
    usize addr = pop(vm).as.function;
    vm->pos = (VmPos){
        .frame_start = vm->stack.len,
        .ip = vm->bytecode.instructions.data + addr
    };
    return FLOW_CONTINUE;
}

static ControlFlow run_enter(Vm* vm) {
    eprintf("TODO: run_enter");
    exit(-1);
    return 0;
}

static ControlFlow run_return(Vm* vm) {
    eprintf("TODO: run_return");
    exit(-1);
    return 0;
}

static ControlFlow run_load_imm(Vm* vm) {
    u64 low = *(vm->pos.ip++);
    u64 high = *(vm->pos.ip++);
    union {
        u64 as_bits;
        Primitive as_primitive;
    } value = {
        .as_bits = (high << 32) | low,
    };
    push(vm, value.as_primitive);
    return FLOW_CONTINUE;
}

static ControlFlow sys_exit(Vm* vm) {
    vm->exit_code = pop(vm).as.integer;
    return FLOW_EXIT;
}

static ControlFlow sys_say_hi(Vm* vm) {
    eprintf("*coughs* hai!!\n");
    return FLOW_CONTINUE;
}

static ControlFlow sys_say_bye(Vm* vm) {
    eprintf("*coughs* bai...\n");
    return FLOW_CONTINUE;
}
