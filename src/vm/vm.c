#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "vm/vm.h"
#include "diagnostic/diagnostic.h"

// must be IEEE 754 float64
typedef double float64_t;

typedef enum control_flow {
    FLOW_EXIT = 0,
    FLOW_CONTINUE = 1,
} control_flow_t;

vm_t new_vm(bytecode_t bytecode, reporter_t* reporter) {
    return (vm_t){
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

void free_vm(vm_t vm) {
    free_array_buf(vm.stack);
}

static control_flow_t run_one(vm_t* vm);

static control_flow_t run_syscall(vm_t* vm);
static control_flow_t run_call(vm_t* vm);
static control_flow_t run_enter(vm_t* vm);
static control_flow_t run_return(vm_t* vm);
static control_flow_t run_load_imm(vm_t* vm);

static control_flow_t sys_exit(vm_t* vm);
static control_flow_t sys_say_hi(vm_t* vm);
static control_flow_t sys_say_bye(vm_t* vm);

int run_vm(vm_t* vm) {
    while (run_one(vm) == FLOW_CONTINUE) {}
    return vm->exit_code;
}

static primitive_t pop(vm_t* vm) {
    primitive_t val;
    array_buf_pop(&vm->stack, &val, primitive_t);
    return val;
}

static void push(vm_t* vm, primitive_t value) {
    array_buf_push(&vm->stack, value);
}

static control_flow_t run_one(vm_t* vm) {
    opcode_t op = (opcode_t)(*vm->pos.ip);
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

static control_flow_t run_syscall(vm_t* vm) {
    syscall_t op = (syscall_t)(*vm->pos.ip);
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

static control_flow_t run_call(vm_t* vm) {
    // FIXME
    assert(sizeof(vm_pos_t) % sizeof(primitive_t) == 0);

    array_buf_extend(
        &vm->stack,
        &vm->pos,
        sizeof(vm_pos_t) / sizeof(primitive_t),
        primitive_t
    );
    size_t addr = pop(vm).as.function;
    vm->pos = (vm_pos_t){
        .frame_start = vm->stack.len,
        .ip = vm->bytecode.instructions.data + addr
    };
    return FLOW_CONTINUE;
}

static control_flow_t run_enter(vm_t* vm) {
    eprintf("TODO: run_enter");
    exit(-1);
    return 0;
}

static control_flow_t run_return(vm_t* vm) {
    eprintf("TODO: run_return");
    exit(-1);
    return 0;
}

static control_flow_t run_load_imm(vm_t* vm) {
    uint64_t low = *(vm->pos.ip++);
    uint64_t high = *(vm->pos.ip++);
    union {
        uint64_t as_bits;
        primitive_t as_primitive;
    } value = {
        .as_bits = (high << 32) | low,
    };
    push(vm, value.as_primitive);
    return FLOW_CONTINUE;
}

static control_flow_t sys_exit(vm_t* vm) {
    vm->exit_code = pop(vm).as.integer;
    return FLOW_EXIT;
}

static control_flow_t sys_say_hi(vm_t* vm) {
    eprintf("*coughs* hai!!\n");
    return FLOW_CONTINUE;
}

static control_flow_t sys_say_bye(vm_t* vm) {
    eprintf("*coughs* bai...\n");
    return FLOW_CONTINUE;
}
