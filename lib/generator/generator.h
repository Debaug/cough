#pragma once

#include "ast/ast.h"
#include "emitter/emitter.h"
#include "diagnostics/report.h"

void generate(Ast* ast, Emitter* emitter);
