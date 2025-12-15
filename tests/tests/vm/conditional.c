#include "tests/common.h"

int main(int argc, const char* argv[]) {
    TestVmSystem vm_system = test_vm_system_new();
    TestReporter reporter = test_reporter_new();

    char const* assembly[] = {
        // call with argument 0 (say hi)
        "   sca 0",
        "   loc :say_message",
        "   cal",
        // call with argument 1 (say bye)
        "   sca 1",
        "   loc :say_message",
        "   cal",
        "",
        "   sca 0",
        "   sys exit",

        // parameters: 0 to say hi, nonzero to say bye.
        // doesn't return anything.
        ":say_message",
        "   jnz :say_bye",
        "   sys hi",
        "   ret",
        ":say_bye",
        "   sys bye",
        "   ret",
    };
    Bytecode bytecode = assemble_parts_or_exit(assembly, sizeof(assembly) / sizeof(char*));

    Vm vm = vm_new((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    vm_run(&vm);

    assert(vm_system.syscalls.len == 3);

    assert(vm_system.syscalls.data[0].kind == SYS_HI);

    assert(vm_system.syscalls.data[1].kind == SYS_BYE);

    assert(vm_system.syscalls.data[2].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[2].as.exit.exit_code == 0);

    return 0;
}
