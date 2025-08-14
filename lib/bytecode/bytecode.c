#include "alloc/alloc.h"
#include "bytecode/bytecode.h"

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
