#include <stdio.h>
#include <inttypes.h>

#include "vm/vm.h"
#include "diagnostic/diagnostic.h"

// must be IEEE 754 float64
typedef double float64_t;

typedef enum control_flow {
    FLOW_EXIT = 0,
    FLOW_CONTINUE = 1,
} control_flow_t;

gc_t new_gc() {
    return (gc_t){
        // .allocations = new_array_buf(),
        // .freed = new_array_buf(),
        // .roots = new_array_buf(),
        .garbage_marker = MARKER_A,
    };
}

vm_t new_vm(bytecode_t bytecode) {
    return (vm_t) {
        .bytecode = bytecode,
        .variable_stack = new_array_buf(uint64_t),
        .variable_frame_index = 0,
        .expression_stack = new_array_buf(uint64_t),
        .ip = bytecode.instructions.data,
        .gc = new_gc(),
    };
}

typedef union value {
    int64_t as_int;
    float64_t as_float;
    void* as_object;
} value_t;

static void push(vm_t* vm, value_t value) {
    array_buf_push(&vm->expression_stack, value);
}

static value_t pop(vm_t* vm) {
    value_t value;
    array_buf_pop(&vm->expression_stack, &value, value_t);
    return value;
}

static control_flow_t run_one(vm_t* vm);

static control_flow_t op_syscall(vm_t* vm);

static control_flow_t op_call(vm_t* vm);
static control_flow_t op_enter(vm_t* vm);
static control_flow_t op_return(vm_t* vm);

static control_flow_t op_scalar(vm_t* vm);
static control_flow_t op_load(vm_t* vm);
static control_flow_t op_store(vm_t* vm);
static control_flow_t op_ignore(vm_t* vm);

static control_flow_t op_add_int(vm_t* vm);

marker_t flip_marker(marker_t marker) {
    return marker ^ 1;
}

void run_vm(vm_t vm) {
    while (run_one(&vm) != FLOW_EXIT);
}

static control_flow_t run_one(vm_t* vm) {
    opcode_t opcode = (opcode_t)(*(vm->ip++));

    switch (opcode) {
    case OP_SYSCALL: return op_syscall(vm);
    
    case OP_CALL: return op_call(vm);
    case OP_ENTER: return op_enter(vm);
    case OP_RETURN: return op_return(vm);

    case OP_SCALAR: return op_scalar(vm);
    case OP_LOAD: return op_load(vm);
    case OP_STORE: return op_store(vm);

    case OP_ADD_INT: return op_add_int(vm);

    default:
        eprintf("invalid instruction 0x%02x", 0);
        return false;
    }
}

static control_flow_t op_syscall(vm_t* vm) {
    syscall_t code = (syscall_t)(*(vm->ip++));
    eprintf("[dbg] syscall %d\n", code);
    switch (code) {
    case SYS_EXIT:
        ;
        int exit_code = (int)(*(vm->ip)++);
        eprintf("exiting with exit code %d\n", exit_code);
        return FLOW_EXIT;

    case SYS_SAY_HI:
        eprintf("hi :)\n");
        return FLOW_CONTINUE;

    case SYS_SAY_BYE:
        eprintf("bye :(\n");
        return FLOW_CONTINUE;

    default:
        eprintf("invalid syscall 0x%02x\n", code);
    }

    return FLOW_EXIT;
}

typedef struct function_state {
    uint64_t variable_frame_index;
    uint64_t ip;
} function_state_t;

static control_flow_t op_call(vm_t* vm) {
    size_t dst_ip = (size_t)(*(vm->ip++));

    function_state_t state = {
        .variable_frame_index = (uint64_t)(vm->variable_frame_index),
        .ip = (uint64_t)(vm->ip),
    };
    array_buf_push(&vm->variable_stack, state);
    vm->variable_frame_index = vm->variable_stack.len;
    vm->ip = vm->bytecode.instructions.data + dst_ip;
    return FLOW_CONTINUE;
}

static control_flow_t op_enter(vm_t* vm) {
    size_t num_variables = (size_t)(*(vm->ip++));

    array_buf_reserve(&vm->variable_stack, num_variables, uint64_t);
    return FLOW_CONTINUE;
}

static control_flow_t op_return(vm_t* vm) {
    // erase variable slots
    vm->variable_stack.len = vm->variable_frame_index;
    // restore previous function state
    function_state_t state;
    array_buf_pop(&vm->variable_stack, &state, function_state_t);
    vm->variable_frame_index = (size_t)state.variable_frame_index;
    vm->ip = (uint32_t*)state.ip;

    eprintf("[dbg] return -> %zi\n", vm->ip - vm->bytecode.instructions.data);

    return FLOW_CONTINUE;
}

static control_flow_t op_scalar(vm_t* vm) {
    uint64_t scalar = (uint64_t)(*(vm->ip++));
    scalar |= ((uint64_t)(*(vm->ip++))) << 32;
    array_buf_push(&vm->expression_stack, scalar);
    return FLOW_CONTINUE;
}

static control_flow_t op_load(vm_t* vm) {
    size_t variable = (size_t)(*(vm->ip++));
    value_t* variable_ptr = (value_t*)(vm->variable_stack.data);
    value_t value = variable_ptr[vm->variable_frame_index + variable];
    push(vm, value);
    return FLOW_CONTINUE;
}

static control_flow_t op_store(vm_t* vm) {
    size_t variable = (size_t)(*(vm->ip++));
    value_t value = pop(vm);
    value_t* variable_ptr = (value_t*)(vm->variable_stack.data);
    variable_ptr[vm->variable_frame_index + variable] = value;
    return FLOW_CONTINUE;
}

static control_flow_t op_ignore(vm_t* vm) {
    pop(vm);
    return FLOW_CONTINUE;
}

static control_flow_t op_add_int(vm_t* vm) {
    int64_t rhs = pop(vm).as_int;
    int64_t lhs = pop(vm).as_int;

    int64_t result;
    if (__builtin_add_overflow(lhs, rhs, &result)) {
        return FLOW_EXIT;
    }
    push(vm, (value_t)result);
    return FLOW_CONTINUE;
}
