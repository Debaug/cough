#pragma once

#include "alloc/array.h"

typedef enum Opcode {
    /// @brief "nop" -- no operation.
    OP_NOP,

    /// @brief "sys" -- execute a given syscall.
    ///
    /// @param syscall The code of the syscall (16-bit immediate).
    OP_SYS,

    /// @brief "frm" -- prepare a new stack frame.
    ///
    /// @param argsz The number of 64-bit words reserved for the arguments (16-bit immediate).
    OP_FRM,

    /// @brief "arg" -- specify the next argument.
    ///
    /// @param arg the part of the argument (64-bit register).
    OP_ARG,

    /// @brief "cas" -- static call.
    ///
    /// @param func the offset of first instruction within the instructions section (64-bit immediate).
    OP_CAS,

    /// @brief "res" -- reserve the specified number of registers as additional stack space.
    ///
    /// @param space the number of registers to be added (16-bit immediate).
    OP_RES,

    /// @brief "ret" -- return from the current function.
    ///
    /// @param val the start of the return value (register).
    /// @param len the number of registers of the return value (16-bit immediate).
    OP_RET,

    /// @brief "sca" -- load a 64-bit constant into a register.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the value to we written (64-bit immediate).
    OP_SCA,
    
    /// @brief "loa" -- load a value into a register.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the pointer to the source 64-bit value (64-bit register).
    OP_LOA,

    /// @brief "sto" -- store a value from a register.
    ///
    /// @param dst the pointer to the destination memory location (64-bit register).
    /// @param src the source 64-bit value (64-bit register).
    OP_STO,

    /// @brief "mov" -- copy the value from a register to another.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the source 64-bit value (64-bit register).
    OP_MOV,

    /// @brief "adu" -- add two `UInt`s together.
    ///
    /// This operation currently simply wraps around on overflow.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param op1 the first source operand (64-bit register).
    /// @param op2 the second source operand (64-bit register).
    OP_ADU,
} Opcode;

typedef enum Syscall {
    SYS_NOP,

    /// @brief "sys exit" -- exit the program with the specified exit code.
    ///
    /// @param exit_code the exit code (64-bit register).
    SYS_EXIT,

    SYS_HI,
    SYS_BYE,

    /// @brief "sys dbg" -- print the content of a register as a `UInt`.
    ///
    /// @param reg the register (64-bit register).
    SYS_DBG,
} Syscall;

typedef u16 Byteword;
typedef ArrayBuf(Byteword) SectionBuf;

typedef struct Bytecode {
    SectionBuf rodata;
    SectionBuf instructions;
} Bytecode;
