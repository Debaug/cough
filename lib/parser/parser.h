#pragma once

#include "diagnostics/report.h"
#include "tokens/token.h"
#include "ast/ast.h"

bool parse(
    TokenStream tokens,
    Reporter* reporter,
    Ast* dst
);
