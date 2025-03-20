#pragma once

#include "alloc/array.h"

// structure of an instruction:
// [opcode] [immediate arguments] [register arguments]

typedef enum Opcode {
    /// @brief "nop" -- no operation.
    OP_NOP,

    /// @brief "syscall" -- calls a function of the VM.
    ///
    /// Immediate parameters:
    /// - Syscall -- the system function to call.
    OP_SYSCALL,

    /// @brief "call" -- calls a function.
    ///
    /// Arguments are on the expression stack, last on top.
    ///
    /// Stack parameters:
    /// - u32 -- the offset of the function from the start of the
    /// instructions section.
    OP_CALL,

    /// @brief "enter" -- initalizes the frame of a function.
    ///
    /// Should be called as the first instruction of a function.
    ///
    /// Immediate parameters:
    /// - u32 -- the number of variables the function shall use.
    OP_ENTER,

    /// @brief "return" -- returns from a function.
    OP_RETURN,

    /// @brief "scalar" -- pushes a scalar constant to the expression stack.
    ///
    /// A scalar is a value of type Int or Float.
    /// Currently, only Int is supported.
    ///
    /// immediate parameters:
    /// - u32 -- the low 32 bits of the scalar
    /// - u32 -- the high 32 bits of the scalar
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
} Opcode;

typedef enum Syscall {
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
} Syscall;

typedef u32 Byteword;
typedef ArrayBuf(Byteword) SectionBuf;

typedef struct Bytecode {
    SectionBuf rodata;
    SectionBuf instructions;
} Bytecode;
