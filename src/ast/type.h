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
    TYPE_STRUCT,
    TYPE_VARIANT,
} element_type_kind_t;

typedef struct element_type {
    element_type_kind_t kind;
    union {
        composite_type_t* composite;
    } as;
} element_type_t;

typedef struct named_type {
    size_t array_depth;
    text_view_t element_type_name;
    element_type_t element_type;
} named_type_t;

typedef struct variable {
    bool mutable;
    named_type_t type;
    text_view_t name;
} variable_t;
typedef array_buf_t(variable_t) variable_array_buf_t;

typedef struct composite_type {
    variable_array_buf_t fields;
} composite_type_t;

parse_result_t parse_type_name(parser_t* parser, named_type_t* dst);
parse_result_t parse_variable(parser_t* parser, variable_t* dst);
parse_result_t parse_struct(parser_t* parser, composite_type_t* dst);
parse_result_t parse_variant(parser_t* parser, composite_type_t* dst);

void debug_named_type(named_type_t type, ast_debugger_t* debugger);
void debug_variable(variable_t variable, ast_debugger_t* debugger);
void debug_struct(composite_type_t struct_, ast_debugger_t* debugger);
void debug_variant(composite_type_t variant, ast_debugger_t* debugger);
