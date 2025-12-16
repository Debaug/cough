#include "tests/common.h"

int main(int argc, const char* argv[]) {
    char const* assembly[] = {
        "   loc :foo",
        "   cal",
        "   sys exit",
        "",
        ":foo",
        "   res 1",
        "   sys hi",
        "   sca 21",
        "   loc :bar",
        "   cal",
        "   set %0",
        "   sys dbg %0",
        "   sca -1",
        "   ret",
        "",
        ":bar",
        "   res 1",
        "   set %0",
        "   var %0",
        "   var %0",
        "   adu",
        "   ret",
    };
    Bytecode bytecode = assembly_to_bytecode(assembly, sizeof(assembly) / sizeof(char*));

    TestVmSystem vm_system = test_vm_system_new();
    TestReporter reporter = test_reporter_new();

    Vm vm = vm_new((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    vm_run(&vm);

    assert(vm_system.syscalls.len == 3);

    assert(vm_system.syscalls.data[0].kind == SYS_HI);

    assert(vm_system.syscalls.data[1].kind == SYS_DBG);
    assert(vm_system.syscalls.data[1].as.dbg.var_idx == 0);
    assert(vm_system.syscalls.data[1].as.dbg.var_val.as_uint == 42);

    assert(vm_system.syscalls.data[2].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[2].as.exit.exit_code == -1);

    return 0;
}
