#include "tests/common.h"

int main(int argc, const char* argv[]) {
    TestVmSystem vm_system = test_vm_system_new();
    TestReporter reporter = test_reporter_new();

    // Byteword instructions[] = {
    //     // entry point, calls main
    //     [0] =
    //             OP_RES, 1,
    //             OP_SCA, 0, [4] = 7, 0, 0, 0,
    //             OP_FRM, 1,
    //             OP_ARG, 0,
    //             OP_CAS, [16] = 64, 0, 0, 0,
    //             OP_SYS, SYS_DBG, 1,
    //             OP_SCA, 0, [28] = 0, 0, 0, 0,
    //             OP_SYS, SYS_EXIT, 0,

    //     // Parameters:
    //     // - %0: the index of the Fibonacci number to compute (F_0 = 0, F_1 = 1)
    //     // Return value: the Fibonacci number, one register.
    //     [64] =
    //             OP_RES, 6,

    //             // if %0 < 2, return %0.
    //             OP_SCA, 1, [68] = 2, 0, 0, 0, // %1 = 2
    //             OP_GEU, 2, 0, 1,
    //             OP_JNZ, [80] = 88, 0, 0, 0, 2,
    //             OP_RET, 0, 1,

    //         [88] =
    //             // set F_0 = 0 and F_1 = 1.
    //             OP_SCA, 2, [92] = 0, 0, 0, 0,
    //             OP_SCA, 3, [100] = 1, 0, 0, 0,

    //             // used to decrement %0.
    //             OP_SCA, 5, [108] = -1, -1, -1, -1,

    //         [112] =
    //             // compute the next Fibonacci number.
    //             OP_ADU, 4, 2, 3,
    //             OP_MOV, 2, 3,
    //             OP_MOV, 3, 4,

    //             // decrement %0.
    //             OP_ADU, 0, 0, 5,

    //             // if %0 >= 2, loop back.
    //             OP_GEU, 4, 0, 1,
    //             OP_JNZ, [132] = 112, 0, 0, 0, 4,

    //             // otherwise, return.
    //             OP_RET, 3, 1,
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
        // entry point, calls fibonacci.
        "   res 1",
        "   sca %0 7",
        "   frm 1",
        "   arg %0",
        "   cas :fibonacci",
        "   sys dbg %1",
        "   sca %0 0",
        "   sys exit %0",
        
        // parameters:
        // - %0: the index of the Fibonacci number to compute (F_0 = 0, F_1 = 1)
        // return value: the Fibonacci number, one register.
        ":fibonacci",
        "   res 5",
        // if index < 2, return index.
        "   sca %1 2",                  // %1 is set to 2 for comparisons with the index at %0.
        "   geu %2 %0 %1",
        "   jnz :loop_start %2",
        "   ret %0 1",
        ":loop_start",
        // set F_0 = 0 and F_1 = 1.
        "   sca %2 0",
        "   sca %3 1",
        "   sca %5 -1",                 // %5 is set to -1 to decrement the index at %0.
        ":loop_body",
        // compute the next Fibonacci number.
        "   adu %4 %2 %3",
        "   mov %2 %3",
        "   mov %3 %4",
        // decrement %0.
        "   adu %0 %0 %5",
        // if %0 >= 2, loop back.
        "   geu %4 %0 %1",
        "   jnz :loop_body %4",
        // otherwise, return.
        "   ret %3 1"
    };
    Bytecode bytecode = assemble_parts_or_exit(assembly, sizeof(assembly) / sizeof(char*));

    Vm vm = vm_new((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    vm_run(&vm);

    assert(vm_system.syscalls.len == 2);

    assert(vm_system.syscalls.data[0].kind == SYS_DBG);
    assert(vm_system.syscalls.data[0].as.dbg.reg_val.as_uint == 13);

    assert(vm_system.syscalls.data[1].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[1].as.exit.exit_code == 0);

    return 0;
}
