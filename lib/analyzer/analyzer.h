#pragma once

#include "ast/ast.h"
#include "diagnostics/report.h"

bool analyze(Ast* ast, Reporter* reporter);
