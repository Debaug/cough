#include <inttypes.h>

#include "disassembler/disassembler.h"

typedef struct Disassembler {
    Bytecode bytecode;
    Byteword const* ip;
    StringBuf* dst;
    ArrayBuf(usize) symbol_locations;
} Disassembler;

static void register_symbols(Disassembler* disassembler) {
    #define REGISTER_LOC_ARG_imb bytecode_read_imb(&disassembler->ip);
    #define REGISTER_LOC_ARG_imw bytecode_read_imw(&disassembler->ip);
    #define REGISTER_LOC_ARG_var bytecode_read_var(&disassembler->ip);
    #define REGISTER_LOC_ARG_loc                                        \
        loc = bytecode_read_loc(&disassembler->ip);               \
        array_buf_push(usize)(&disassembler->symbol_locations, loc);
    #define REGISTER_LOC_ARG(i, kind) REGISTER_LOC_ARG_##kind

    #define REGISTER_LOC_CASE(code, mnemo, ...)             \
        case code:                                          \
            FOR_ALL(REGISTER_LOC_ARG __VA_OPT__(, __VA_ARGS__)) \
            break;
    
    Byteword const* end =
        disassembler->bytecode.instructions.data
        + disassembler->bytecode.instructions.len;
    while (disassembler->ip < end) {
        Opcode opcode = bytecode_read_opcode(&disassembler->ip);
        usize loc;
        switch (opcode) {
        FOR_OPERATIONS(REGISTER_LOC_CASE)

        case OP_SYS:;
            Syscall syscall = bytecode_read_syscall(&disassembler->ip);
            switch (syscall) {
            FOR_SYSCALLS(REGISTER_LOC_CASE)

            default:
                exit(-1);
                break;
            }
            break;
        
        default:
            // unreachable
            exit(-1);
            break;
        }
    }
}

static void disassemble_arg_imb(Disassembler* disassembler) {
    Byteword val = bytecode_read_imb(&disassembler->ip);
    char buf[64];
    usize len = snprintf(buf, 64, " %"  PRIu16, val);
    string_buf_extend_slice(
        disassembler->dst,
        (String){ .data = buf, .len = len }
    );
}

static void disassemble_arg_imw(Disassembler* disassembler) {
    Word val = bytecode_read_imw(&disassembler->ip);
    char buf[64];
    usize len = snprintf(buf, 64, " %"  PRIu64, val.as_uint);
    string_buf_extend_slice(
        disassembler->dst,
        (String){ .data = buf, .len = len }
    );
}

static void disassemble_arg_loc(Disassembler* disassembler) {
    usize val = bytecode_read_loc(&disassembler->ip);
    char buf[64];
    usize len = snprintf(buf, 64, " :s%zu", val);
    string_buf_extend_slice(
        disassembler->dst,
        (String){ .data = buf, .len = len }
    );
}

static void disassemble_arg_var(Disassembler* disassembler) {
    usize val = bytecode_read_var(&disassembler->ip);
    char buf[64];
    usize len = snprintf(buf, 64, " %%%zu", val);
    string_buf_extend_slice(
        disassembler->dst,
        (String){ .data = buf, .len = len }
    );
}

static void disassemble_one(Disassembler* disassembler) {
    #define DISASSEMBLE_ARG(i, kind, ...) disassemble_arg_##kind(disassembler);
    #define DISASSEMBLE_ONE_CASE(code, mnemo, ...)              \
        case code:                                              \
            FOR_ALL(DISASSEMBLE_ARG __VA_OPT__(, __VA_ARGS__))  \
            break;

    Opcode opcode = bytecode_read_opcode(&disassembler->ip);
    string_buf_extend(disassembler->dst, "    ");
    string_buf_extend(disassembler->dst, instruction_mnemonics[opcode].chars);
    switch (opcode) {
    FOR_OPERATIONS(DISASSEMBLE_ONE_CASE)
    
    case OP_SYS:;
        Syscall syscall = bytecode_read_syscall(&disassembler->ip);
        string_buf_push(disassembler->dst, ' ');
        string_buf_extend(disassembler->dst, syscall_mnemonics[syscall].chars);
        
        switch (syscall) {
        FOR_SYSCALLS(DISASSEMBLE_ONE_CASE)
        
        default:
            // unreachable
            exit(-1);
            break;
        }

    default:
        // unreachable
        exit(-1);
        break;
    }

    string_buf_push(disassembler->dst, '\n');
}

StringBuf disassemble(Bytecode bytecode) {
    StringBuf dst = string_buf_new();
    Disassembler disassembler = {
        .bytecode = bytecode,
        .ip = bytecode.instructions.data,
        .dst = &dst,
        .symbol_locations = array_buf_new_usize(),
    };
    register_symbols(&disassembler);
    disassembler.ip = disassembler.bytecode.instructions.data;
    usize symbol_location_index = 0;
    Byteword const* end = bytecode.instructions.data + bytecode.instructions.len;
    while (true) {
        if (symbol_location_index < disassembler.symbol_locations.len) {
            usize pos = disassembler.ip - disassembler.bytecode.instructions.data;
            usize next_symbol = disassembler.symbol_locations.data[symbol_location_index];
            if (next_symbol == pos) {
                symbol_location_index++;
                char buf[64];
                usize len = snprintf(buf, 64, ":s%zu\n", next_symbol);
                string_buf_extend_slice(&dst, (String){ .data = buf, .len = len });
            }
        }

        if (disassembler.ip == end) {
            break;
        }
        disassemble_one(&disassembler);
    }
    return dst;
}
