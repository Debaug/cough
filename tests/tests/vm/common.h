#pragma once

#include "diagnostics/diagnostics.h"
#include "vm/system.h"

typedef struct TestReporter {
    Reporter base;
    I32ArrayBuf error_codes;
} TestReporter;

TestReporter new_test_reporter(void);
void free_test_reporter(TestReporter reporter);

typedef struct SyscallRecord {
    Syscall kind;
    union {
        struct {
            i64 exit_code;
        } exit;
        struct {
            usize reg_idx;
            Word reg_val;
        } dbg;
    } as;
} SyscallRecord;
typedef ArrayBuf(SyscallRecord) SyscallRecordArrayBuf;

typedef struct TestVmSystem {
    VmSystem base;
    SyscallRecordArrayBuf syscalls;
} TestVmSystem;

TestVmSystem new_test_vm_system(void);
void free_test_vm_system(TestVmSystem system);
