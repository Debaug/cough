#include "tests/common.h"

int main(int argc, const char* argv[]) {
    TestVmSystem vm_system = new_test_vm_system();
    TestReporter reporter = new_test_reporter();

    Byteword instructions[] = {
        // entry point, calls main
        [0] =
                OP_RES, 1,
                OP_SCA, 0, [4] = 7, 0, 0, 0,
                OP_FRM, 1,
                OP_ARG, 0,
                OP_CAS, [16] = 64, 0, 0, 0,
                OP_SYS, SYS_DBG, 1,
                OP_SCA, 0, [28] = 0, 0, 0, 0,
                OP_SYS, SYS_EXIT, 0,

        // Parameters:
        // - %0: the index of the Fibonacci number to compute (F_0 = 0, F_1 = 1)
        // Return value: the Fibonacci number, one register.
        [64] =
                OP_RES, 5,

                // if %0 < 2, return %0.
                OP_SCA, 1, [68] = 2, 0, 0, 0, // %1 = 2
                OP_GEU, 2, 0, 1,
                OP_JNZ, [80] = 88, 0, 0, 0, 2,
                OP_RET, 0, 1,

            [88] =
                // set F_0 = 0 and F_1 = 1.
                OP_SCA, 2, [92] = 0, 0, 0, 0,
                OP_SCA, 3, [100] = 1, 0, 0, 0,

                // used to decrement %0.
                OP_SCA, 5, [108] = -1, -1, -1, -1,

            [112] =
                // compute the next Fibonacci number.
                OP_ADU, 4, 2, 3,
                OP_MOV, 2, 3,
                OP_MOV, 3, 4,

                // decrement %0.
                OP_ADU, 0, 0, 5,

                // if %0 >= 2, loop back.
                OP_GEU, 4, 0, 1,
                OP_JNZ, [132] = 112, 0, 0, 0, 4,

                // otherwise, return.
                OP_RET, 3, 1,
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

    assert(vm_system.syscalls.len == 2);

    assert(vm_system.syscalls.data[0].kind == SYS_DBG);
    assert(vm_system.syscalls.data[0].as.dbg.reg_val.as_uint == 13);

    assert(vm_system.syscalls.data[1].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[1].as.exit.exit_code == 0);

    return 0;
}
