#include "tests/vm/common.h"

static void test_reporter_start(Reporter* raw, Severity severity, int code) {
    TestReporter* self = (TestReporter*)raw;
    i32 code_i32 = code;
    array_buf_push(&self->error_codes, code_i32);
}

static void test_reporter_end(Reporter* self) {}

static void test_reporter_message(Reporter* self, StringBuf message) {}

static void test_reporter_source_code(Reporter* self, TextView source_code) {}

static usize test_reporter_n_errors(const Reporter* raw) {
    const TestReporter* self = (const TestReporter*)raw;
    return self->error_codes.len;
}

static const ReporterVTable test_reporter_vtable = {
    .start = test_reporter_start,
    .end = test_reporter_end,
    .message = test_reporter_message,
    .source_code = test_reporter_source_code,
    .n_errors = test_reporter_n_errors,
};

TestReporter new_test_reporter(void) {
    return (TestReporter){
        .base.vtable = &test_reporter_vtable,
        .error_codes = new_array_buf()
    };
}

void free_test_reporter(TestReporter reporter) {
    free_array_buf(reporter.error_codes);
}

static void test_vm_system_nop(VmSystem* raw) {
    TestVmSystem* self = (TestVmSystem*)raw;
    SyscallRecord record = { .kind = SYS_NOP };
    array_buf_push(&self->syscalls, record);
}

static void test_vm_system_exit(VmSystem* raw, i64 exit_code) {
    TestVmSystem* self = (TestVmSystem*)raw;
    SyscallRecord record = {
        .kind = SYS_EXIT,
        .as.exit = { .exit_code = exit_code },
    };
    array_buf_push(&self->syscalls, record);
}

static void test_vm_system_hi(VmSystem* raw) {
    TestVmSystem* self = (TestVmSystem*)raw;
    SyscallRecord record = { .kind = SYS_HI };
    array_buf_push(&self->syscalls, record);
}

static void test_vm_system_bye(VmSystem* raw) {
    TestVmSystem* self = (TestVmSystem*)raw;
    SyscallRecord record = { .kind = SYS_BYE };
    array_buf_push(&self->syscalls, record);
}

static void test_vm_system_dbg(VmSystem* raw, usize reg_idx, Word reg_val) {
    TestVmSystem* self = (TestVmSystem*)raw;
    SyscallRecord record = {
        .kind = SYS_DBG,
        .as.dbg = {
            .reg_idx = reg_idx,
            .reg_val = reg_val,
        },
    };
    array_buf_push(&self->syscalls, record);
}

static const VmSystemVTable test_vm_system_vtable = {
    .nop =  test_vm_system_nop,
    .exit = test_vm_system_exit,
    .hi =   test_vm_system_hi,
    .bye =  test_vm_system_bye,
    .dbg =  test_vm_system_dbg,
};

TestVmSystem new_test_vm_system(void) {
    return (TestVmSystem){
        .base.vtable = &test_vm_system_vtable,
        .syscalls = new_array_buf(),
    };
}

void free_test_vm_system(TestVmSystem system) {
    free_array_buf(system.syscalls);
}
