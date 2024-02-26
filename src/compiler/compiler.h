#pragma once

#include "util/array.h"

// structure of an instruction:
// [opcode] [immediate arguments] [register arguments]

typedef enum opcode {
    OP_NOP,

    // call a function of the VM.
    //
    // immediate parameters:
    // - syscall_t (uint32_t) -- the system function to call.
    OP_SYSCALL,

    // call a function.
    //
    // arguments are on the expression stack, last on top.
    //
    // immediate parameters:
    // - uint32_t -- the offset of the function from the start of the
    // instructions section.
    OP_CALL,

    // initalize the frame of a function.
    //
    // should be called as the first instruction of a function.
    //
    // immediate parameters:
    // - uint32_t -- the number of variables the function shall use.
    OP_ENTER,

    // return from a function.
    OP_RETURN,

    // push a scalar constant to the expression stack.
    //
    // a scalar is a value of type Int or Float.
    //
    // immediate parameters:
    // - uint32_t -- the low 32 bits of the scalar
    // - uint32_t -- the high 32 bits of the scalar
    OP_SCALAR,

    // push the value of a variable to the expression stack.
    OP_LOAD,

    // store a value into a variable.
    OP_STORE,

    // erase the top value from the expression stack.
    OP_IGNORE,

    // add two values of type Int.
    // crashes on overflow.
    //
    // register parameters:
    // - Int, Int -- the terms of the addition
    // - Int -- the destination register
    OP_ADD_INT,
} opcode_t;

typedef enum syscall {
    SYS_NOP,

    // exit the program.
    //
    // register parameters:
    // - Int -- the exit code (will be truncated to fit into a C int)
    SYS_EXIT,

    SYS_SAY_HI,
    SYS_SAY_BYE,

    // write a value of type String to a File.
    //
    // register parameters:
    // - File -- the file to be written; this may also be stdout or stderr.
    // - String -- the string to be printed.
    SYS_WRITE_STRING,
} syscall_t;

typedef size_t vaddr_t;

typedef array_buf_t(uint32_t) instruction_buf_t;

typedef struct bytecode {
    instruction_buf_t instructions;
} bytecode_t;
