#include <inttypes.h>

#include "diagnostics/log.h"
#include "vm/system.h"

static void sys_nop(VmSystem* self) {}

static void sys_exit(VmSystem* self, i64 exit_code) {
    eprintf(
        "[sys exit] program finished with exit code %" PRId64 " (0x%" PRIx64 ")\n",
        exit_code, exit_code
    );
}

static void sys_hi(VmSystem* self) {
    eprintf("[sys hi]   Cough says hi!\n");
}

void sys_bye(VmSystem* self) {
    eprintf("[sys bye]  Cough says bye!\n");
}

void sys_dbg(VmSystem* self, usize reg_idx, Word reg_val) {
    eprintf(
        "[sys dbg]  %zu: %" PRIu64 " (0x%" PRIx64 ")\n",
        reg_idx, reg_val.as_uint, reg_val.as_uint
    );
}

static const VmSystemVTable default_vm_system_vtable = {
    .nop = sys_nop,
    .exit = sys_exit,
    .hi = sys_hi,
    .bye = sys_bye,
    .dbg = sys_dbg,
};

DefaultVmSystem new_default_vm_system(void) {
    return (DefaultVmSystem){
        .base.vtable = &default_vm_system_vtable
    };
}

void free_default_vm_system(DefaultVmSystem system) {}
