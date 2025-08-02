#pragma once

#include "compiler/ast/symbol.h"
#include "alloc/array.h"
#include "alloc/alloc_stack.h"

typedef struct Scope Scope;
typedef ArrayBuf(Scope) ScopeArrayBuf;

typedef struct Scope {
    SymbolArrayBuf symbols;
    ScopeArrayBuf children;
    Scope* parent;
} Scope;

const Symbol* find_symbol(Scope scope, StringView name);
const Symbol* find_symbol_of(Scope scope, StringView name, SymbolKind mask);
bool add_symbol(Scope* scope, Symbol symbol);

typedef struct Analyzer {
    Scope* root;
    Scope* current_scope;
    Reporter* reporter;
} Analyzer;

Analyzer new_analyzer(Reporter* reporter);
void free_analyzer(Analyzer analyzer);

Scope* analyzer_enter_new_scope(Analyzer* analyzer);
void analyzer_exit_scope(Analyzer* analyzer);
