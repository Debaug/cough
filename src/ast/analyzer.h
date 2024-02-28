#pragma once

#include "ast/symbol.h"
#include "alloc/array.h"
#include "alloc/alloc_stack.h"

typedef struct scope scope_t;
typedef array_buf_t(scope_t) scope_array_buf_t;

typedef struct scope {
    symbol_array_buf_t symbols;
    scope_array_buf_t children;
    scope_t* parent;
} scope_t;

typedef struct analyzer {
    scope_t* root;
    scope_t* current_scope;
} analyzer_t;

analyzer_t new_analyzer(void);
void free_analyzer(analyzer_t analyser);

scope_t* analyzer_enter_new_scope(analyzer_t* analyzer);
void analyzer_exit_scope(analyzer_t* analyzer);
