#pragma once

#include "collections/string.h"
#include "bytecode/bytecode.h"
#include "diagnostics/result.h"
#include "diagnostics/report.h"

Result assemble(String assembly, Reporter* reporter, Bytecode* dst);
Bytecode assemble_parts_or_exit(char const** parts, usize count);
