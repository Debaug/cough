#include "tests/common.h"

int main(int argc, const char* argv[]) {
    TestVmSystem vm_system = test_vm_system_new();
    TestReporter reporter = test_reporter_new();

    char const* assembly[] = {
        // entry point, calls fibonacci.
        "   res 1",
        "   sca 7",
        "   loc :fibonacci",
        "   cal",
        "   set %0",
        "   sys dbg %0",
        "   sca 0",
        "   sys exit",
        
        // parameters:
        // - the index of the Fibonacci number to compute (F_0 = 0, F_1 = 1)
        // return value: the Fibonacci number
        ":fibonacci",
        "   res 3",
        "   set %0",        // %0 contains the index
        // if index <= 1, return index.
        "   var %0",
        "   sca 1",
        "   gtu",
        "   jnz :loop_start",
        "   var %0",
        "   ret",
        ":loop_start",
        // initalize with F_0 = 0 and F_1 = 1.
        "   sca 0",
        "   set %1",
        "   sca 1",
        "   set %2",
        ":loop_body",
        // compute the next Fibonacci number.
        "   var %1",
        "   var %2",
        "   adu",
        "   var %2",
        "   set %1",
        "   set %2",
        // decrement %0.
        "   var %0",
        "   sca -1",
        "   adu",
        "   set %0",
        // if %0 >= 2, loop back.
        "   var %0",
        "   sca 2",
        "   geu",
        "   jnz :loop_body",
        // otherwise, return.
        "   var %2",
        "   ret"
    };
    Bytecode bytecode = assemble_parts_or_exit(assembly, sizeof(assembly) / sizeof(char*));

    Vm vm = vm_new((VmSystem*)&vm_system, bytecode, (Reporter*)&reporter);
    vm_run(&vm);

    assert(vm_system.syscalls.len == 2);

    assert(vm_system.syscalls.data[0].kind == SYS_DBG);
    eprintf("val: %d", (int)vm_system.syscalls.data[0].as.dbg.var_val.as_uint);
    assert(vm_system.syscalls.data[0].as.dbg.var_val.as_uint == 13);

    assert(vm_system.syscalls.data[1].kind == SYS_EXIT);
    assert(vm_system.syscalls.data[1].as.exit.exit_code == 0);

    return 0;
}
