#include "tests/common.h"

int main(int argc, const char* argv[]) {
    char const* assembly[] = {
        "   frm 0",
        "   cas :foo",
        "   sys exit %0",
        "",
        ":foo",
        "   res 1",
        "   sca %0 21",
        "   sys hi",
        "   frm 1",
        "   arg %0",
        "   cas :bar",
        "   sys dbg %1",
        "   sca %0 -1",
        "   ret %0 1"
        "",
        ":bar",
        "   res 1",
        "   mov %1 %0",
        "   adu %0 %0 %1",
        "   ret %0 1"
    };
    Bytecode bytecode = assemble_parts_or_exit(assembly, sizeof(assembly) / sizeof(char*));

    TestVmSystem vm_system = test_vm_system_new();
    TestReporter reporter = test_reporter_new();

    Vm vm = vm_new((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    vm_run(&vm);

    assert(vm_system.syscalls.len == 3);

    assert(vm_system.syscalls.data[0].kind == SYS_HI);

    assert(vm_system.syscalls.data[1].kind == SYS_DBG);
    assert(vm_system.syscalls.data[1].as.dbg.reg_idx == 1);
    assert(vm_system.syscalls.data[1].as.dbg.reg_val.as_uint == 42);

    assert(vm_system.syscalls.data[2].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[2].as.exit.exit_code == -1);

    return 0;
}
