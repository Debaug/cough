#include <string.h>

#include "tests/common.h"
#include "assembler/assembler.h"

int main(int argc, char const *argv[]) {
    char const* assembly[] = {
        "   res 1",
        "   sca 10",
        "   set %0",
        "   var %0",
        "   sca 32",
        "   loc :add",
        "   cal",
        "   set %0",
        "   sys dbg %0",
        "",
        "   sca 0",
        "   sys exit",
        "",
        ":add",
        "   adu",
        "   ret"
    };
    Bytecode bytecode =
        assembly_to_bytecode(assembly, sizeof(assembly) / sizeof(char*));

    Byteword expected[] = {
            [0] =   OP_RES, 1,
            [2] =   OP_SCA, [4] = 10, 0, 0, 0,
            [8] =   OP_SET, 0,
            [10] =  OP_VAR, 0,
            [12] =  OP_SCA, [16] = 32, 0, 0, 0,
            [20] =  OP_LOC, [24] = 38, 0, 0, 0,
            [28] =  OP_CAL,
            [29] =  OP_SET, 0,
            [31] =  OP_SYS, SYS_DBG, 0,
            
            [34] =  OP_SCA, [36] = 0, 0, 0, 0,
            [40] =  OP_SYS, SYS_EXIT,

        [42] =
                    OP_ADU,
            [43] =  OP_RET
    };

    assert(bytecode.instructions.len == sizeof(expected) / sizeof(Byteword));
    assert(!memcmp(bytecode.instructions.data, expected, sizeof(expected) / sizeof(Byteword)));

    return 0;
}
