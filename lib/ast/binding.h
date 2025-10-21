#pragma once

#include "collections/array.h"
#include "ast/expression.h"

typedef struct VariableBinding {
    TypeId type;
} VariableBinding;

typedef struct ConstantBinding {
    TypeId type;
    ExpressionId value;
} ConstantBinding;

typedef struct TypeOrConstantBinding {
    bool is_type;
    union {
        TypeId type;
        ConstantBinding constant;
    };
} TypeOrConstantBinding;

DECL_ARRAY_BUF(VariableBinding)
DECL_ARRAY_BUF(TypeOrConstantBinding)

typedef enum BindingKind {
    BINDING_TYPE,
    BINDING_CONSTANT,
    BINDING_VARIABLE,
} BindingKind;

typedef struct Binding {
    BindingKind kind;
    Identifier identifier;
    union {
        TypeId type;
        VariableBinding variable;
        ConstantBinding constant;
    } as;
} Binding;

typedef usize ScopeId;

typedef struct Scope {
    ArrayBuf(TypeOrConstantBinding) _types_and_constants;
    ArrayBuf(VariableBinding) _variables;
} Scope;

DECL_ARRAY_BUF(Scope)

typedef struct ScopeGraph {
    ArrayBuf(Scope) _scopes;
} ScopeGraph;

ScopeGraph scope_graph_new(void);
void scope_graph_free(ScopeGraph* graph);

typedef struct ScopeNew {
    Scope* scope;
    ScopeId id;
} ScopeNew;

ScopeNew scope_new(ScopeGraph* graph);
Scope const* scope_get(ScopeGraph graph, ScopeId id);
Scope* scope_get_mut(ScopeGraph* graph, ScopeId);

Binding scope_get_binding(Scope const* scope, BindingId id);
BindingId scope_insert_constant(Scope* scope, Identifier identifier, ConstantBinding constant);
