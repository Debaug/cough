#include "tests/vm/common.h"
#include "tests/vm/tests.h"

int test_vm_conditional(void) {
    TestVmSystem vm_system = new_test_vm_system();
    TestReporter reporter = new_test_reporter();

    // // computes the 42th Fibonacci number (F_0 = 0, F_1 = 1) and debugs it.
    // Byteword instructions[] = {
    //     [0] =
    //             OP_FRM, 0,
    //             OP_CAS, [4] = 64, 0, 0, 0,
    //             OP_SCA, 0, [12] = 0, 0, 0, 0,
    //             OP_SYS, SYS_EXIT, 0,
        
    //     [64] =
    //             OP_RES, 1,
    //             OP_SCA, 0, [68] = 0, 0, 0, 0,  // the index of the Fibonacci number to compute.
    //             OP_FRM, 1,
    //             OP_ARG, 0,
    //             OP_CAS, [80] = 128, 0, 0, 0,
    //             OP_SYS, SYS_DBG, 1,
    //             OP_SCA, 0, [92] = 0, 0, 0, 0,
    //             OP_RET, 0, 1,

    //     // parameter %0 holds the index of the Fibonacci number to compute.
    //     [128] =
    //             OP_RES, 4,

    //             // return idx if idx <= 1 (in these cases, F_i = i).
    //             OP_SCA, 1, 1,
    //             OP_GTU, 1, 0, 1,
    //             OP_JNZ, [140] = 160, 0, 0, 0, 1,
    //             OP_RET, 0, 1,
            
    //         [160] =
    //             // set the first numbers to 0 and 1.
    //             OP_SCA, 1, [164] = 0, 0, 0, 0,
    //             OP_SCA, 2, [172] = 1, 0, 0, 0,
            
    //         [178] =
    //             // if idx < 2, stop.
    //             OP_SCA, 4, [180] = 2, 0, 0, 0,
    //             OP_GTU, 3, 4, 0,
    //             OP_JNZ, 3, [192] = 240, 0, 0, 0,

    //             // compute next value & shift variables.
    //             OP_ADU, 3, 1, 2,
    //             OP_MOV, 1, 2,
    //             OP_MOV, 2, 3,

    //             // decrement idx
    //             OP_SCA, 3, [208] = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    //             OP_ADU, 0, 0, 3,

    //             // loop back
    //             OP_JMP, [220] = 178, 0, 0, 0,
            
    //         [240] =
    //             OP_RET, 2, 1
    // };

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
