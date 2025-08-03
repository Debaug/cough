#include "tests/vm/common.h"
#include "tests/vm/tests.h"

int test_vm_function_call() {
    TestVmSystem vm_system = new_test_vm_system();
    TestReporter reporter = new_test_reporter();

    Byteword instructions[] = {
        [0] =
            OP_FRM, 0,
            OP_CAS, [4] = 64, 0, 0, [8] =
            OP_SYS, SYS_EXIT, 0,
        
        [64] =
            OP_RES, 1,
            OP_SCA, 0, [68] = 21, 0, 0, 0, [72] =
            OP_SYS, SYS_HI,
            OP_FRM, 1,
            OP_ARG, 0,
            OP_CAS, [80] = 128, 0, 0, 0, [84] =
            OP_SYS, SYS_DBG, 1,
            OP_SCA, 0, [92] = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, [96] =
            OP_RET, 0, 1,

        [128] =
            OP_RES, 1,
            OP_MOV, 1, 0,
            OP_ADU, 0, 0, 1,
            OP_RET, 0, 1,
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

    assert(vm_system.syscalls.data[1].kind == SYS_DBG);
    assert(vm_system.syscalls.data[1].as.dbg.reg_idx == 1);
    assert(vm_system.syscalls.data[1].as.dbg.reg_val.as_uint == 42);

    assert(vm_system.syscalls.data[2].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[2].as.exit.exit_code == -1);

    return 0;
}
