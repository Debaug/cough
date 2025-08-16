#pragma once

#include "primitives/primitives.h"
#include "bytecode/bytecode.h"
#include "alloc/hash_map.h"
#include "diagnostics/diagnostics.h"

DECL_HASH_MAP(StringView, usize)
DECL_HASH_MAP(Mnemonic, u8)

typedef struct SymbolRef {
    TextView symbol;
    usize ref_in_bytecode;
    usize symbol_in_assembly;
} SymbolRef;
typedef ArrayBuf(SymbolRef) SymbolRefArrayBuf;

typedef struct Assembler {
    HashMap(Mnemonic, u8) mnemonic_instructions;
    HashMap(Mnemonic, u8) mnemonic_syscalls;
    const char* input;
    Reporter* reporter;
    bool error;
    usize pos;
    HashMap(StringView, usize) symbols;
    SymbolRefArrayBuf symbol_refs;
    Bytecode output;
} Assembler;

Assembler new_assembler(const char* assembly, Reporter* reporter);
Bytecode assemble(Assembler* assembler);
void free_assembler(Assembler assembler);
