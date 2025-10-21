#pragma once

#include "collections/array.h"
#include "ast/type.h"
#include "ast/binding.h"
#include "ast/expression.h"
#include "ast/storage.h"

typedef struct Module {
    ScopeId global_scope;
    ArrayBuf(ConstantDef) global_constants;
} Module;

typedef struct Ast {
    TypeRegistry types;
    ScopeGraph scopes;
    ArrayBuf(Expression) expressions;
    
    Module root;
} Ast;
