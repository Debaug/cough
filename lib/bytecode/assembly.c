#include "bytecode/assembly.h"

Assembler new_assembler(
    const char* path,
    const char* paragraphs[],
    usize paragraph_count,
    Reporter* reporter
);
Bytecode assemble(Assembler assembler);
