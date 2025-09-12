#pragma once

#include <stdalign.h>

#include "collections/array.h"
#include "ops/eq.h"
#include "ops/hash.h"

typedef union Word {
    void const* as_ptr;
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
    
    /// @brief `loc` -- load a location into a register.
    ///
    /// @param dst the destination register (64-bit register).
    /// @param src the location to be written (64-bit immediate).
    OP_LOC,
    
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

    OPCODES_LEN,
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

    SYSCALLS_LEN,
} Syscall;

DECL_ARRAY_BUF(Byteword);

typedef struct Bytecode {
    ArrayBuf(Byteword) rodata;
    ArrayBuf(Byteword) instructions;
} Bytecode;

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
#define bytecode_read_loc bytecode_read_location

void bytecode_write_opcode(Bytecode* bytecode, Opcode opcode);
void bytecode_write_syscall(Bytecode* bytecode, Syscall syscall);
void bytecode_write_byteword(Bytecode* bytecode, Byteword byteword);
void bytecode_write_word(Bytecode* bytecode, Word word);
void bytecode_write_register_index(Bytecode* bytecode, usize register_index);
void bytecode_write_location(Bytecode* bytecode, usize symbol);
void bytecode_write_location_at(Byteword** ip, usize symbol);

#define bytecode_write(kind) bytecode_write_##kind
#define bytecode_write_sys bytecode_write_syscall
#define bytecode_write_imb bytecode_write_byteword
#define bytecode_write_imw bytecode_write_word
#define bytecode_write_preg bytecode_write_register_index
#define bytecode_write_reg bytecode_write_register_index
#define bytecode_write_loc bytecode_write_location

#define FOR_OPERATIONS(proc)            \
    proc(OP_NOP, nop)                   \
    proc(OP_FRM, frm, imb)              \
    proc(OP_ARG, arg, reg)              \
    proc(OP_CAS, cas, loc)              \
    proc(OP_RES, res, imb)              \
    proc(OP_RET, ret, preg, imb)        \
    proc(OP_SCA, sca, preg, imw)        \
    proc(OP_LOC, loc, preg, loc)        \
    proc(OP_LOA, loa, preg, reg)        \
    proc(OP_STO, sto, reg, reg)         \
    proc(OP_MOV, mov, preg, reg)        \
    proc(OP_JMP, jmp, loc)              \
    proc(OP_JNZ, jnz, loc, reg)         \
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

#define FOR_ALL(proc, ...)      \
    __VA_OPT__(FOR_ALL1(proc, __VA_ARGS__))
#define FOR_ALL1(proc, a, ...)  \
    proc(0, a __VA_OPT__(,0)) __VA_OPT__(FOR_ALL2(proc, __VA_ARGS__))
#define FOR_ALL2(proc, b, ...)  \
    proc(1, b __VA_OPT__(,0)) __VA_OPT__(FOR_ALL3(proc, __VA_ARGS__))
#define FOR_ALL3(proc, c, ...)  \
    proc(2, c)

typedef struct Mnemonic {
    alignas(u64) char chars[8];
} Mnemonic;
bool eq(Mnemonic)(Mnemonic a, Mnemonic b);
void hash(Mnemonic)(Hasher* hasher, Mnemonic mnemo);

extern Mnemonic instruction_mnemonics[OPCODES_LEN];
extern Mnemonic syscall_mnemonics[SYSCALLS_LEN];
