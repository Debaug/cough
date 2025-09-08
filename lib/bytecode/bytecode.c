#include "bytecode/bytecode.h"
#include "ops/ptr.h"

IMPL_ARRAY_BUF(Byteword);

Opcode bytecode_read_opcode(const Byteword** ip) {
    return (Opcode)bytecode_read_byteword(ip);
}

Syscall bytecode_read_syscall(const Byteword** ip) {
    return (Syscall)bytecode_read_byteword(ip);
}

Byteword bytecode_read_byteword(const Byteword** ip) {
    return *((*ip)++);
}

Word bytecode_read_word(const Byteword** ip) {
    const Word* ip_word = (const Word*)align_up(*ip, alignof(Word));
    Word val = *(ip_word++);
    *ip = (const Byteword*)ip_word;
    return val;
}

usize bytecode_read_register_index(const Byteword** ip) {
    return bytecode_read_byteword(ip);
}

usize bytecode_read_symbol(const Byteword** ip) {
    return bytecode_read_word(ip).as_uint;
}

void bytecode_write_opcode(Bytecode* bytecode, Opcode opcode) {
    bytecode_write_byteword(bytecode, opcode);
}

void bytecode_write_syscall(Bytecode* bytecode, Syscall syscall) {
    bytecode_write_byteword(bytecode, syscall);
}

void bytecode_write_byteword(Bytecode* bytecode, Byteword byteword) {
    array_buf_push(Byteword)(&bytecode->instructions, byteword);
}

void bytecode_write_word(Bytecode* bytecode, Word word) {
    ArrayBuf(Byteword) instructions = bytecode->instructions;
    while ((instructions.len * sizeof(Byteword)) % alignof(Word) != 0) {
        Byteword zero = 0;
        array_buf_push(Byteword)(&instructions, zero);
    }
    array_buf_extend(Byteword)(
        &instructions,
        &word,
        sizeof(Word) / sizeof(Byteword)
    );
    bytecode->instructions = instructions;
}

void bytecode_write_register_index(Bytecode* bytecode, usize register_index) {
    // FIXME: check that the register index fits in a byteword
    bytecode_write_byteword(bytecode, (Byteword)register_index);
}

void bytecode_write_symbol(Bytecode* bytecode, usize symbol) {
    bytecode_write_word(bytecode, (Word){ .as_uint = symbol });
}

bool eq(Mnemonic)(Mnemonic a, Mnemonic b) {
    return *(const u64*)&a == *(const u64*)&b;
}

void hash(Mnemonic)(Hasher* hasher, Mnemonic mnemo) {
    hash(u64)(hasher, *(const u64*)&mnemo);
}

#define INSTRUCTION_MNEMONIC(opcode, mnemo, ...) [opcode] = #mnemo,
Mnemonic instruction_mnemonics[] = {
    FOR_INSTRUCTIONS(INSTRUCTION_MNEMONIC)
};

#define SYSCALL_MNEMONIC(syscall, mnemo, ...) [syscall] = #mnemo,
Mnemonic syscall_mnemonics[] = {
    FOR_SYSCALLS(SYSCALL_MNEMONIC)
};
