#pragma once

#include "alloc/array.h"

typedef union Word {
    const void* as_ptr;
    void* as_mut_ptr;
    u64 as_uint;
    i64 as_int;
    f64 as_float;
} Word;
typedef u16 Byteword;

typedef enum Opcode {
    /// @brief `nop` -- no operation.
    OP_NOP = 0,

    /// @brief `sys` -- execute a given syscall.
    ///
    /// @param syscall The code of the syscall (16-bit immediate).
    OP_SYS,

    /// @brief `frm` -- prepare a new stack frame.
    ///
    /// @param argsz The number of 64-bit words reserved for the arguments (16-bit immediate).
    OP_FRM,

    /// @brief `arg` -- specify the next argument.
    ///
    /// @param arg the part of the argument (64-bit register).
    OP_ARG,

    /// @brief `cas` -- static call.
    ///
    /// @param func the location of the first instruction of the function (64-bit immediate).
    /// Is usually a `frm` instruction.
    OP_CAS,

    /// @brief `res` -- reserve the specified number of registers as additional stack space.
    ///
    /// @param space the number of registers to be added (16-bit immediate).
    OP_RES,

    /// @brief `ret` -- return from the current function.
    ///
    /// @param val the start of the return value (register).
    /// @param len the number of registers of the return value (16-bit immediate).
    OP_RET,

    /// @brief `sca` -- load a 64-bit constant into a register.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the value to be written (64-bit immediate).
    OP_SCA,
    
    /// @brief `loa` -- load a value into a register.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the pointer to the source 64-bit value (64-bit register).
    OP_LOA,

    /// @brief `sto` -- store a value from a register.
    ///
    /// @param dst the pointer to the destination memory location (64-bit register).
    /// @param src the source 64-bit value (64-bit register).
    OP_STO,

    /// @brief `mov` -- copy the value from a register to another.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the source 64-bit value (64-bit register).
    OP_MOV,

    /// @brief `jmp` -- jump to the specified memory location.
    ///
    /// @param loc The location of the instruction to jump to.
    OP_JMP,

    /// @brief `jnz` -- jump to the specified memory location a register is non-zero.
    ///
    /// @param loc The location of the instruction to jump to.
    /// @param reg The register to check.
    OP_JNZ,

    /// @brief `equ` -- check if two `UInt`'s are equal.
    ///
    /// Writes `true` to `dst` if `op1 == op2`. Otherwise, writes `false`.
    ///
    /// @param dst The destination register (64-bit register).
    /// @param op1 The first operand register (64-bit register).
    /// @param op2 The second operand register (64-bit register).
    OP_EQU,

    /// @brief `neu` -- check if two `UInt`'s are distinct.
    ///
    /// Writes `true` to `dst` if `op1 != op2`. Otherwise, writes `false`.
    ///
    /// @param dst The destination register (64-bit register).
    /// @param op1 The first operand register (64-bit register).
    /// @param op2 The second operand register (64-bit register).
    OP_NEU,

    /// @brief `geu` -- check if one `UInt` is not less than another.
    ///
    /// Writes `true` to `dst` if `op1 >= op2`. Otherwise, writes `false`.
    ///
    /// @param dst The destination register (64-bit register).
    /// @param op1 The first operand register (64-bit register).
    /// @param op2 The second operand register (64-bit register).
    OP_GEU,

    /// @brief `gtu` -- check if two `UInt`'s are equal.
    ///
    /// Writes `true` to `dst` if `op1 > op2`. Otherwise, writes `false`.
    ///
    /// @param dst The destination register (64-bit register).
    /// @param op1 The first operand register (64-bit register).
    /// @param op2 The second operand register (64-bit register).
    OP_GTU,

    /// @brief `adu` -- add two `UInt`s together.
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

    /// @brief `sys exit` -- exit the program with the specified exit code.
    ///
    /// @param exit_code the exit code (64-bit register).
    SYS_EXIT,

    SYS_HI,
    SYS_BYE,

    /// @brief `sys dbg` -- print the content of a register as a `UInt`.
    ///
    /// @param reg the register (64-bit register).
    SYS_DBG,
} Syscall;

Opcode bytecode_read_opcode(const Byteword** ip);
Syscall bytecode_read_syscall(const Byteword** ip);
Byteword bytecode_read_byteword(const Byteword** ip);
Word bytecode_read_word(const Byteword** ip);
usize bytecode_read_register_index(const Byteword** ip);
usize bytecode_read_symbol(const Byteword** ip);

#define bytecode_read(kind) bytecode_read_##kind
#define bytecode_read_sys bytecode_read_syscall
#define bytecode_read_imb bytecode_read_byteword
#define bytecode_read_imw bytecode_read_word
#define bytecode_read_preg bytecode_read_register_index
#define bytecode_read_reg bytecode_read_register_index
#define bytecode_read_sym bytecode_read_symbol

#define FOR_INSTRUCTIONS(proc)          \
    proc(OP_NOP, nop)                   \
    proc(OP_SYS, sys, sys)              \
    proc(OP_FRM, frm, imb)              \
    proc(OP_ARG, arg, reg)              \
    proc(OP_CAS, cas, sym)              \
    proc(OP_RES, res, imb)              \
    proc(OP_RET, ret, preg, imb)        \
    proc(OP_SCA, sca, preg, imw)        \
    proc(OP_LOA, loa, preg, reg)        \
    proc(OP_STO, sto, reg, reg)         \
    proc(OP_MOV, mov, preg, reg)        \
    proc(OP_JMP, jmp, sym)              \
    proc(OP_JNZ, jnz, sym, reg)         \
    proc(OP_EQU, equ, preg, reg, reg)   \
    proc(OP_NEU, neu, preg, reg, reg)   \
    proc(OP_GEU, geu, preg, reg, reg)   \
    proc(OP_GTU, gtu, preg, reg, reg)   \
    proc(OP_ADU, adu, preg, reg, reg)   \

#define FOR_SYSCALLS(proc)      \
    proc(SYS_NOP, nop)          \
    proc(SYS_EXIT, exit, reg)   \
    proc(SYS_HI, hi)            \
    proc(SYS_BYE, bye)          \
    proc(SYS_DBG, dbg, preg)    \

#define FOR_ARGS(proc, ...) \
    __VA_OPT__(FOR_ARGS1(proc, __VA_ARGS__))
#define FOR_ARGS1(proc, arg, ...)   \
    proc(arg __VA_OPT__(,0)) __VA_OPT__(FOR_ARGS2(proc, __VA_ARGS__))
#define FOR_ARGS2(proc, arg, ...)   \
    proc(arg __VA_OPT__(,0)) __VA_OPT__(FOR_ARGS3(proc, __VA_ARGS__))
#define FOR_ARGS3(proc, arg) proc(arg)

#define PROCESS_INSTRUCTION_CASE_ARG(kind, ...) \
    FETCH_ARG(kind) __VA_OPT__(,)

#define PROCESS_INSTRUCTION_CASE(op, mnemo, ...)                                \
    case op:                                                                    \
        OP(                                                                     \
            mnemo,                                                              \
            FOR_ARGS(PROCESS_INSTRUCTION_CASE_ARG __VA_OPT__(, __VA_ARGS__))    \
        );                                                                      \
        break;                                                                  \

#define PROCESS_INSTRUCTION(opcode) do {        \
    switch (opcode) {                           \
    FOR_INSTRUCTIONS(PROCESS_INSTRUCTION_CASE)  \
    }                                           \
} while(0)

#define PROCESS_SYSCALL_CASE(sys, mnemo, ...)                                   \
    case sys:                                                                   \
        SYS(                                                                    \
            mnemo,                                                              \
            FOR_ARGS(PROCESS_INSTRUCTION_CASE_ARG __VA_OPT__(, __VA_ARGS__))    \
        );                                                                      \
        break;                                                                  \

#define PROCESS_SYSCALL(syscall) do {   \
    switch (syscall) {                  \
    FOR_SYSCALLS(PROCESS_SYSCALL_CASE)  \
    }                                   \
} while(0)

typedef ArrayBuf(Byteword) SectionBuf;

typedef struct Bytecode {
    SectionBuf rodata;
    SectionBuf instructions;
} Bytecode;
