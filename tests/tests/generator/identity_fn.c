#include "tests/common.h"
#include "disassembler/disassembler.h"

int main(int argc, char const** argv) {
    char const source[] = 
        "identity :: fn x: Bool -> Bool => x;\n"
    ;
    char const* assembly[] = {
        "res 1",
        "set %0",
        "var %0",
        "ret",
    };
    Bytecode generated = source_to_bytecode(STRING_LITERAL(source));
    Bytecode expected = assembly_to_bytecode(assembly, sizeof(assembly) / sizeof(char const*));
    
    assert(generated.instructions.len == expected.instructions.len);
    assert(memcmp(generated.instructions.data, expected.instructions.data, expected.instructions.len) == 0);
}
