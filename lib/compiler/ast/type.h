#pragma once

#include <stddef.h>

#include "text/text.h"
#include "compiler/ast/parser.h"
#include "compiler/ast/debug.h"
#include "alloc/array.h"
#include "diagnostics/diagnostics.h"

typedef struct Analyzer Analyzer;

typedef struct CompositeType CompositeType;

typedef enum ElementTypeKind {
    TYPE_NEVER,
    TYPE_UNIT,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_FUNCTION,
    TYPE_STRUCT,
    TYPE_VARIANT
} ElementTypeKind;

typedef struct FunctionSignature FunctionSignature;
typedef struct ElementType {
    ElementTypeKind kind;
    union {
        const CompositeType* composite;
        const FunctionSignature* function_signature;
    } as;
} ElementType;

typedef struct Type {
    ElementType element_type;
    usize array_depth;
} Type;

typedef struct NamedType {
    Type type;
    TextView element_type_name;
} NamedType;

typedef struct Variable {
    NamedType type;
    TextView name;
    usize offset;
    usize size;
    bool mutable;
} Variable;
typedef ArrayBuf(Variable) VariableArrayBuf;

typedef struct Field {
    NamedType type;
    TextView name;
    usize offset;
    usize size;
} Field;
typedef ArrayBuf(Field) FieldArrayBuf;

typedef struct FunctionSignature {
    VariableArrayBuf parameters;
    bool has_return_type : 1;
    NamedType return_type;
} FunctionSignature;

typedef struct CompositeType {
    FieldArrayBuf fields;
} CompositeType;

bool type_eq(Type a, Type b);
bool function_signature_eq(FunctionSignature a, FunctionSignature b);

Result parse_type_name(Parser* parser, NamedType* dst);
Result parse_variable(Parser* parser, Variable* dst);
Result parse_function_signature(Parser* parser, FunctionSignature* dst);
Result parse_struct(Parser* parser, CompositeType* dst);
Result parse_variant(Parser* parser, CompositeType* dst);

Result analyze_type(Analyzer* analyzer, NamedType* type);
Result analyze_function_signature(
    Analyzer* analyzer,
    FunctionSignature* signature
);

const Field* find_field(CompositeType type, StringView name);

void debug_type(Type type, AstDebugger* debugger);
void debug_named_type(NamedType type, AstDebugger* debugger);
void debug_field(Field field, AstDebugger* debugger);
void debug_variable(Variable variable, AstDebugger* debugger);
void debug_function_signature(FunctionSignature signature, AstDebugger* debugger);
void debug_struct(CompositeType struct_, AstDebugger* debugger);
void debug_variant(CompositeType variant, AstDebugger* debugger);
