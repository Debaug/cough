#pragma once

#include "primitives/primitives.h"
#include "bytecode/bytecode.h"
#include "alloc/hash_map.h"
#include "diagnostics/diagnostics.h"

DECL_HASH_MAP(StringView, usize)

typedef struct Assembler {
    struct {
        const char* path;
        const char** paragraphs;
        usize paragraph_count;
    } input;
    Reporter* reporter;
    struct {
        usize paragraph;
        usize token;
    } pos;
    HashMap(StringView, usize) symbols;
    Bytecode output;
} Assembler;

Assembler new_assembler(
    const char* path,
    const char* paragraphs[],
    usize paragraph_count,
    Reporter* reporter
);
Bytecode assemble(Assembler assembler);
