#pragma once

#include "collections/array.h"
#include "ast/type.h"

typedef struct Identifier {
    Range range;
} Identifier;

typedef struct BindingId {
    usize id : sizeof(usize) - 1;
    bool is_variable : 1;
} BindingId;

DECL_ARRAY_BUF(BindingId);

typedef enum TypeNameKind {
    TYPE_NAME_IDENTIFIER
} TypeNameKind;

typedef struct TypeName {
    TypeNameKind kind;
    union {
        Identifier identifier;
    } as;
    Range range;
} TypeName;

typedef usize ExpressionId;

typedef struct ConstantDef {
    Identifier name;
    bool explicitly_typed;
    TypeName type_name;
    TypeId type;
    ExpressionId value;
    BindingId binding;
} ConstantDef;

DECL_ARRAY_BUF(ConstantDef);

typedef struct VariableDef {
    Identifier name;
    bool explicitly_typed;
    TypeName type_name;
    TypeId type;
    BindingId binding;
} VariableDef;

typedef enum PatternKind {
    PATTERN_VARIABLE,
} PatternKind;

typedef struct Pattern {
    PatternKind kind;
    union {
        VariableDef variable;
    } as;
    bool explicitly_typed;
    TypeName type_name;
    TypeId type;
    Range range;
} Pattern;

typedef struct Function {
    Pattern input;
    bool explicit_output_type;
    TypeName output_type_name;
    TypeId output_type;
    ExpressionId output;
} Function;

typedef struct VariableRef {
    Identifier identifier;
    BindingId binding;
} VariableRef;

typedef enum ExpressionKind {
    EXPRESSION_VARIABLE,
    EXPRESSION_FUNCTION,
    EXPRESSION_LITERAL_BOOL,
} ExpressionKind;

typedef struct Expression {
    ExpressionKind kind;
    union {
        VariableRef variable;
        Function function;
        bool literal_bool;
    } as;
    Range range;
    TypeId type;
} Expression;

DECL_ARRAY_BUF(Expression);
