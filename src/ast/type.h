#pragma once

#include <stddef.h>

#include "text/text.h"
#include "ast/parser.h"
#include "ast/debug.h"
#include "alloc/array.h"

typedef struct composite_type composite_type_t;

typedef enum element_type_kind {
    TYPE_NEVER,
    TYPE_UNIT,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_FUNCTION,
    TYPE_STRUCT,
    TYPE_VARIANT
} element_type_kind_t;

typedef struct function_signature function_signature_t;
typedef struct element_type {
    element_type_kind_t kind;
    union {
        const composite_type_t* composite;
        const function_signature_t* function_signature;
    } as;
} element_type_t;

typedef struct type {
    size_t array_depth;
    element_type_t element_type;
} type_t;

typedef struct named_type {
    type_t type;
    text_view_t element_type_name;
} named_type_t;

typedef struct variable {
    named_type_t type;
    text_view_t name;
    size_t offset;
    size_t size;
    bool mutable;
} variable_t;
typedef array_buf_t(variable_t) variable_array_buf_t;

typedef struct field {
    named_type_t type;
    text_view_t name;
    size_t offset;
    size_t size;
} field_t;
typedef array_buf_t(field_t) field_array_buf_t;

typedef struct function_signature {
    variable_array_buf_t parameters;
    bool has_return_type : 1;
    named_type_t return_type;
} function_signature_t;

typedef struct composite_type {
    field_array_buf_t fields;
} composite_type_t;

bool type_eq(type_t a, type_t b);
bool function_signature_eq(function_signature_t a, function_signature_t b);

parse_result_t parse_type_name(parser_t* parser, named_type_t* dst);
parse_result_t parse_variable(parser_t* parser, variable_t* dst);
parse_result_t parse_function_signature(parser_t* parser, function_signature_t* dst);
parse_result_t parse_struct(parser_t* parser, composite_type_t* dst);
parse_result_t parse_variant(parser_t* parser, composite_type_t* dst);

analyze_result_t analyze_type(analyzer_t* analyzer, named_type_t* type);
analyze_result_t analyze_function_signature(
    analyzer_t* analyzer,
    function_signature_t* signature
);

const field_t* find_field(composite_type_t type, string_view_t name);

void debug_type(type_t type, ast_debugger_t* debugger);
void debug_field(field_t field, ast_debugger_t* debugger);
void debug_variable(variable_t variable, ast_debugger_t* debugger);
void debug_function_signature(function_signature_t signature, ast_debugger_t* debugger);
void debug_struct(composite_type_t struct_, ast_debugger_t* debugger);
void debug_variant(composite_type_t variant, ast_debugger_t* debugger);
