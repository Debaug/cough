#include "tests/common.h"

int main(int argc, const char* argv[]) {
    TestVmSystem vm_system = test_vm_system_new();
    TestReporter reporter = test_reporter_new();

    // Byteword instructions[] = {
    //     [0] =
    //             OP_RES, 2,
    //             OP_SCA, 0, [4] = 0, 0, 0, 0,
    //             OP_SCA, 1, [12] = 1, 0, 0, 0,

    //             // call with argument 0 (say hi)
    //             OP_FRM, 1,
    //             OP_ARG, 0,
    //             OP_CAS, [24] = 64, 0, 0, 0,

    //             // call with argument 1 (say bye)
    //             OP_FRM, 1,
    //             OP_ARG, 1,
    //             OP_CAS, [36] = 64, 0, 0, 0,

    //             OP_SYS, SYS_EXIT, 0,

    //     // param: %0: 0 to say hi, nonzero to say bye.
    //     // doesn't return anything.
    //     [64] =
    //             OP_RES, 2,
                
    //             // check if %0 == 0.
    //             OP_SCA, 1, [68] = 0, 0, 0, 0,
    //             OP_EQU, 2, 0, 1,
    //             OP_JNZ, [80] = 92, 0, 0, 0, 2,

    //             // if %0 != 0, say bye.
    //             OP_SYS, SYS_BYE,
    //             OP_JMP, [88] = 94, 0, 0, 0,

    //         [92] =
    //             // if %0 == 0, say hi.
    //             OP_SYS, SYS_HI,

    //         [94] =
    //             OP_RET, 0, 0
    // };

    // ArrayBuf(Byteword) instruction_buf = array_buf_new(Byteword)();
    // array_buf_extend(Byteword)(
    //     &instruction_buf,
    //     instructions,
    //     sizeof(instructions) / sizeof(Byteword)
    // );

    // Bytecode bytecode = {
    //     .instructions = instruction_buf,
    //     .rodata = array_buf_new(Byteword)(),
    // };

    char const* assembly[] = {
        "   res 2",
        "   sca %0 0",
        "   sca %1 1",
        // call with argument 0 (say hi)
        "   frm 1",
        "   arg %0",
        "   cas :say_message",
        // call with argument 1 (say bye)
        "   frm 1",
        "   arg %1",
        "   cas :say_message",
        "",
        "   sys exit %0",

        // parameters: %0: 0 to say hi, nonzero to say bye.
        // doesn't return anything.
        ":say_message",
        "   res 2",
        // check if %0 == 0.
        "   sca %1 0",
        "   equ %2 %0 %1",
        "   jnz :say_hi %2",
        // if %0 != 0, say bye.
        "   sys bye",
        "   jmp :end",
        // if %0 == 0, say hi.
        ":say_hi",
        "   sys hi",
        // return.
        ":end",
        "   ret %0 0",
    };
    Bytecode bytecode = assemble_parts_or_exit(assembly, sizeof(assembly) / sizeof(char*));
    assert(sizeof(assembly) != 0);
    assert(bytecode.instructions.data != NULL);

    Vm vm = vm_new((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    vm_run(&vm);

    assert(vm_system.syscalls.len == 3);

    assert(vm_system.syscalls.data[0].kind == SYS_HI);

    assert(vm_system.syscalls.data[1].kind == SYS_BYE);

    assert(vm_system.syscalls.data[2].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[2].as.exit.exit_code == 0);

    return 0;
}
