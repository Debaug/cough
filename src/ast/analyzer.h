#pragma once

#include "ast/symbol.h"
#include "alloc/array.h"
#include "alloc/alloc_stack.h"

typedef struct scope scope_t;
typedef array_buf_t(scope_t) scope_array_buf_t;

typedef struct scope {
    symbol_array_buf_t symbols;
    scope_array_buf_t children;
} scope_t;

typedef struct analyzer {
    scope_t root;
    alloc_stack_t* alloc_stack;
} analyzer_t;
