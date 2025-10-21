#include "ast/binding.h"

IMPL_ARRAY_BUF(TypeOrConstantBinding)
IMPL_ARRAY_BUF(VariableBinding)
IMPL_ARRAY_BUF(Scope)

ScopeGraph scope_graph_new(void) {
    return (ScopeGraph){ ._scopes = array_buf_new(Scope)() };
}

void scope_graph_free(ScopeGraph* graph) {
    for (usize i = 0; i < graph->_scopes.len; i++) {
        Scope scope = graph->_scopes.data[i];
        array_buf_free(TypeOrConstantBinding)(&scope._types_and_constants);
        array_buf_free(VariableBinding)(&scope._variables);
    }
    array_buf_free(Scope)(&graph->_scopes);
}
