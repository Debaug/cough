#pragma once

#include "primitives/primitives.h"
#include "bytecode/bytecode.h"
#include "collections/array.h"
#include "collections/hash_map.h"
#include "diagnostics/diagnostics.h"

DECL_HASH_MAP(String, usize)
DECL_HASH_MAP(Mnemonic, u8)

typedef struct SymbolRef {
    Range symbol;
    usize ref_in_bytecode;
    usize symbol_in_assembly;
} SymbolRef;
DECL_ARRAY_BUF(SymbolRef)

typedef struct Assembler {
    HashMap(Mnemonic, u8) mnemonic_instructions;
    HashMap(Mnemonic, u8) mnemonic_syscalls;
    const char* input;
    Reporter* reporter;
    bool error;
    usize pos;
    HashMap(String, usize) symbols;
    ArrayBuf(SymbolRef) symbol_refs;
    Bytecode output;
} Assembler;

Assembler new_assembler(const char* assembly, Reporter* reporter);
Bytecode assemble(Assembler* assembler);
void free_assembler(Assembler assembler);
