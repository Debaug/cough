#pragma once

#include "alloc/array.h"

// structure of an instruction:
// [opcode] [immediate arguments] [register arguments]

typedef enum opcode {
    /// @brief "nop" -- no operation.
    OP_NOP,

    /// @brief "syscall" - calls a function of the VM.
    ///
    /// Immediate parameters:
    /// - syscall_t -- the system function to call.
    OP_SYSCALL,

    /// @brief "call" -- calls a function.
    ///
    /// Arguments are on the expression stack, last on top.
    ///
    /// Stack parameters:
    /// - uint32_t -- the offset of the function from the start of the
    /// instructions section.
    OP_CALL,

    /// @brief "enter" -- initalizes the frame of a function.
    ///
    /// Should be called as the first instruction of a function.
    ///
    /// Immediate parameters:
    /// - uint32_t -- the number of variables the function shall use.
    OP_ENTER,

    /// @brief "return" -- returns from a function.
    OP_RETURN,

    /// @brief "scalar" -- pushes a scalar constant to the expression stack.
    ///
    /// A scalar is a value of type Int or Float.
    /// Currently, only Int is supported.
    ///
    /// immediate parameters:
    /// - uint32_t -- the low 32 bits of the scalar
    /// - uint32_t -- the high 32 bits of the scalar
    OP_LOAD_IMM,

    /// @brief "load" -- pushes the value of a variable to the expression stack.
    OP_LOAD_VAR,

    /// @brief "store" -- stores a value into a variable.
    OP_STORE,

    /// @brief "ignore" -- erases the top value from the expression stack.
    OP_IGNORE,

    /// @brief "addi" --  add two values of type Int.
    ///
    /// crashes on overflow.
    ///
    /// expression parameters:
    /// - Int, Int -- the terms of the addition
    /// - Int -- the destination register
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

typedef uint32_t byteword_t;
typedef array_buf_t(byteword_t) section_buf_t;

typedef struct bytecode {
    section_buf_t rodata;
    section_buf_t instructions;
} bytecode_t;
