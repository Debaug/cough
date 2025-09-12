#include <string.h>

#include "tests/common.h"
#include "assembler/assembler.h"

int main(int argc, char const *argv[]) {
    char const* assembly[] = {
        "   res 2",
        "   sca %0 10",
        "   sca %1 32",
        "",
        "   frm 2",
        "   arg %0",
        "   arg %1",
        "   cas :add",
        "   sys dbg %2",
        "",
        "   sca %0 0",
        "   sys exit %0",
        "",
        ":add",
        "   adu %0 %0 %1",
        "   ret %0 1"
    };
    Bytecode bytecode =
        assemble_parts_or_exit(assembly, sizeof(assembly) / sizeof(char*));

    Byteword expected[] = {
            [0] =   OP_RES, 2,
            [2] =   OP_SCA, 0, [4] = 10, 0, 0, 0,
            [8] =   OP_SCA, 1, [12] = 32, 0, 0, 0,

            [16] =  OP_FRM, 2,
            [18] =  OP_ARG, 0,
            [20] =  OP_ARG, 1,
            [22] =  OP_CAS, [24] = 43, 0, 0, 0,
            [28] =  OP_SYS, SYS_DBG, 2,

            [31] =  OP_SCA, 0, [36] = 0, 0, 0, 0,
            [40] =  OP_SYS,  SYS_EXIT, 0,

        [43] =
                    OP_ADU, 0, 0, 1,
            [47] =  OP_RET, 0, 1
    };
    usize expected_len = 50;

    assert(bytecode.instructions.len == expected_len);
    assert(!memcmp(bytecode.instructions.data, expected, expected_len));

    return 0;
}
