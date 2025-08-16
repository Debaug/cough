#include <ctype.h>

#include "bytecode/assembly.h"

IMPL_HASH_MAP(StringView, usize)
IMPL_HASH_MAP(Mnemonic, u8)

Assembler new_assembler(
    const char* assembly,
    Reporter* reporter
) {
    HashMap(Mnemonic, u8) mnemonic_instructions = new_hash_map(Mnemonic, u8)();
    for (usize i = 0; i < OPCODES_LEN; i++) {
        hash_map_insert(Mnemonic, u8)(&mnemonic_instructions, instruction_mnemonics[i], i);
    }

    HashMap(Mnemonic, u8) mnemonic_syscalls = new_hash_map(Mnemonic, u8)();
    for (usize i = 0; i < SYSCALLS_LEN; i++) {
        hash_map_insert(Mnemonic, u8)(&mnemonic_syscalls, syscall_mnemonics[i], i);
    }

    return (Assembler){
        .mnemonic_instructions = mnemonic_instructions,
        .mnemonic_syscalls = mnemonic_syscalls,
        .input = assembly,
        .reporter = reporter,
        .error = false,
        .pos = 0,
        .symbols = new_hash_map(StringView, usize),
        .output = (Bytecode){
            .instructions = new_array_buf(),
            .rodata = new_array_buf(),
        },
    };
}

static void assemble_line(Assembler* assembler);
static Result assemble_instruction(Assembler* assembler);
static Result assemble_symbol(Assembler* assembler);
static Result resolve_symbols(Assembler* assembler);

static void skip_whitespace(Assembler* assembler);
static void skip_whitespace_same_line(Assembler* assembler);

Bytecode assemble(Assembler* assembler) {

}

static void assemble_line(Assembler* assembler) {
    const char* assembly = assembler->input;
    while (isspace(assembly[assembler->pos])) {
        assembler->pos++;
    }

    Result result;
    if (assembly[assembler->pos] == ':') {
        result = assemble_symbol(assembler);
    } else {
        result = assemble_instruction(assembler);
    }

    if (result != SUCCESS) {
        char c;
        while (c = assembly[assembler->pos], c != '\n' || c == '\0') {
            assembler->pos++;
        }
    }
}

static void skip_whitespace(Assembler* assembler) {
    const char* assembly = assembler->input;
    char c;
    while (c = assembly[assembler->pos], isspace(c)) {
        assembler->pos++;
    }
}

static void skip_whitespace_same_line(Assembler* assembler) {
    const char* assembly = assembler->input;
    char c;
    while (c = assembly[assembler->pos], isspace(c) && c != '\n') {
        assembler->pos++;
    }
}

static Result assemble_symbol(Assembler* assembler) {
    // skip ':' character
    assembler->pos++;

    usize start_idx = assembler->pos;
    const char* name = assembler->input + start_idx;
    char c = name[0];
    if (!isalpha(c) || c != '_') {
        // TODO: invalid symbol name
        return ERROR;
    }

    usize len = 1;
    while (name[len] != '\0') {
        if (!isalnum(name[len])) break;
        len++;
    }
}

static Result assemble_sys(Assembler* assembler);
static Result assemble_imb(Assembler* assembler);
static Result assemble_imw(Assembler* assembler);
static Result assemble_preg(Assembler* assembler);
static Result assemble_reg(Assembler* assembler);
static Result assemble_sym(Assembler* assembler);

static Result assemble_instruction(Assembler* assembler) {
    const char* assembly = assembler->input;
    Mnemonic mnemo = { .chars = { 0, 0, 0, 0, 0, 0, 0, 0 } };
    char c;
    usize i = 0;
    for (; c = assembly[assembler->pos], !isspace(c) && i < 8; assembler->pos++, i++) {
        mnemo.chars[i] = c;
    }
    if (i >= 8) {
        // TODO: invalid mnemonic
        return ERROR;
    }

    const Opcode* p_opcode = hash_map_get(Mnemonic, u8)(assembler->mnemonic_instructions, mnemo);
    if (p_opcode == NULL) {
        // TODO: invalid mnemonic
        return ERROR;
    }
    Opcode opcode = *p_opcode;

    #define FETCH_ARG(kind)

    return SUCCESS;
}

static Result process_sys(Assembler* assembler) {
    // TODO
}

static Result parse_uint(Assembler* assembler, u64* dst) {
    const char* assembly = assembler->input;
    // FIXME: overflow
    u64 n = 0;
    bool empty = true;
    while (true) {
        char c = assembly[assembler->pos];
        if (!('0' <= c && c <= '9')) {
            break;
        }
        empty = false;
        n *= 10;
        n += c - '0';
    }
    if (empty || !isspace(assembly[assembler->pos])) {
        return ERROR;
    }
    *dst = n;
    return SUCCESS;
}

static Result parse_immediate(Assembler* assembler, Word* dst) {
    const char* assembly = assembler->input;
    if (assembly[assembler->pos] != '#') {
        // TODO: invalid immediate value
        return ERROR;
    }
    assembler->pos++;

    if (parse_uint(assembler, &dst->as_uint) != SUCCESS) {
        // TODO: invalid immediate value
        return ERROR;
    }

    return SUCCESS;
}

static Result assemble_imb(Assembler* assembler) {
    Word immediate;
    if (parse_immediate(assembler, &immediate) != SUCCESS) {
        // TODO: invalid immediate
        return ERROR;
    }
    if (!assembler->error) {
        bytecode_write(imb)(&assembler->output, immediate.as_uint);
    }
    return SUCCESS;
}

static Result assemble_imw(Assembler* assembler) {
    Word immediate;
    if (parse_immediate(assembler, &immediate) != SUCCESS) {
        // TODO: invalid immediate
        return ERROR;
    }
    if (!assembler->error) {
        bytecode_write(imw)(&assembler->output, immediate);
    }
    return SUCCESS;
}

static Result assemble_preg(Assembler* assembler) {
    const char* assembly = assembler->input;
    if (assembly[assembler->pos] != '%') {
        // TODO: invalid immediate value
        return ERROR;
    }
    assembler->pos++;

    u64 index;
    if (parse_uint(assembler, &index) != SUCCESS) {
        // TODO: invalid immediate value
        return ERROR;
    }

    // FIXME: overflow
    if (!assembler->error) {
        bytecode_write(preg)(&assembler->output, index);
    }

    return SUCCESS;
}

static Result assemble_reg(Assembler* assembler) {
    return assemble_preg(assembler);
}

static Result assemble_sym(Assembler* assembler) {
    exit(1);
    return ERROR;
}
