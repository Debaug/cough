#include "tests/vm/common.h"
#include "tests/vm/tests.h"

int test_vm_conditional(void) {
    TestVmSystem vm_system = new_test_vm_system();
    TestReporter reporter = new_test_reporter();

    Byteword instructions[] = {
        [0] =
                OP_RES, 2,
                OP_SCA, 0, [4] = 0, 0, 0, 0,
                OP_SCA, 1, [12] = 1, 0, 0, 0,

                // call with argument 0 (say hi)
                OP_FRM, 1,
                OP_ARG, 0,
                OP_CAS, [24] = 64, 0, 0, 0,

                // call with argument 1 (say bye)
                OP_FRM, 1,
                OP_ARG, 1,
                OP_CAS, [36] = 64, 0, 0, 0,

                OP_SYS, SYS_EXIT, 0,

        // param: %0: 0 to say hi, nonzero to say bye.
        // doesn't return anything.
        [64] =
                OP_RES, 2,
                
                // check if %0 == 0.
                OP_SCA, 1, [68] = 0, 0, 0, 0,
                OP_EQU, 2, 0, 1,
                OP_JNZ, [80] = 92, 0, 0, 0, 2,

                // if %0 != 0, say bye.
                OP_SYS, SYS_BYE,
                OP_JMP, [88] = 94, 0, 0, 0,

            [92] =
                // if %0 == 0, say hi.
                OP_SYS, SYS_HI,

            [94] =
                OP_RET, 0, 0
    };

    SectionBuf instruction_buf = new_array_buf();
    array_buf_extend(
        &instruction_buf,
        &instructions,
        sizeof(instructions) / sizeof(Byteword)
    );

    Bytecode bytecode = {
        .instructions = instruction_buf,
        .rodata = new_array_buf(),
    };

    Vm vm = new_vm((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    run_vm(&vm);

    assert(vm_system.syscalls.len == 3);

    assert(vm_system.syscalls.data[0].kind == SYS_HI);

    assert(vm_system.syscalls.data[1].kind == SYS_BYE);

    assert(vm_system.syscalls.data[2].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[2].as.exit.exit_code == 0);

    return 0;
}
