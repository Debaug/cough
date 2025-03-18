#pragma once

#include "text/text.h"
#include "ast/type.h"

typedef enum SymbolKind {
    SYMBOL_TYPE = 1,
    SYMBOL_FUNCTION = 2,
    SYMBOL_VARIABLE = 4,
} SymbolKind;

typedef struct Function Function;

typedef struct Symbol {
    TextView name;
    SymbolKind kind;
    union {
        ElementType type;
        Function* function;
        Variable* variable;
    } as;
} Symbol;

typedef ArrayBuf(Symbol) SymbolArrayBuf;
