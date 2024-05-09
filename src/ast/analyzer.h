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

const symbol_t* find_symbol(scope_t scope, string_view_t name);
const symbol_t* find_symbol_of(scope_t scope, string_view_t name, symbol_kind_t mask);
bool add_symbol(scope_t* scope, symbol_t symbol);

typedef struct analyzer {
    scope_t* root;
    scope_t* current_scope;
} analyzer_t;

typedef enum analyze_result {
    ANALYZE_SUCCESS,
    ANALYZE_ERROR,
} analyze_result_t;

analyzer_t new_analyzer(void);
void free_analyzer(analyzer_t analyzer);

scope_t* analyzer_enter_new_scope(analyzer_t* analyzer);
void analyzer_exit_scope(analyzer_t* analyzer);

analyze_result_t analyze_variable(analyzer_t* analyzer, variable_t* variable);
analyze_result_t analyze_field(analyzer_t* analyzer, field_t* variable);
analyze_result_t analyze_composite(analyzer_t* analyzer, composite_type_t* variable);
