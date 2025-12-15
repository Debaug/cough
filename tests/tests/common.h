#pragma once

#include <assert.h>

#include "collections/array.h"
#include "diagnostics/report.h"
#include "bytecode/bytecode.h"
#include "assembler/assembler.h"
#include "vm/system.h"
#include "vm/vm.h"

typedef struct TestReporter {
    Reporter base;
    ArrayBuf(i32) error_codes;
} TestReporter;

TestReporter test_reporter_new(void);
void test_reporter_free(TestReporter reporter);

typedef struct SyscallRecord {
    Syscall kind;
    union {
        struct {
            i64 exit_code;
        } exit;
        struct {
            usize var_idx;
            Word var_val;
        } dbg;
    } as;
} SyscallRecord;
DECL_ARRAY_BUF(SyscallRecord)

typedef struct TestVmSystem {
    VmSystem base;
    ArrayBuf(SyscallRecord) syscalls;
} TestVmSystem;

TestVmSystem test_vm_system_new(void);
void test_vm_system_free(TestVmSystem system);
