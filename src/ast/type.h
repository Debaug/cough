#pragma once

#include <stddef.h>

#include "text/text.h"
#include "ast/parser.h"
#include "ast/debug.h"

typedef size_t type_t;
#define TYPE_UNRESOLVED ((type_t)(-1))

// primitive types
#define TYPE_UNIT ((type_t)0)
#define TYPE_BOOL ((type_t)1)
#define TYPE_INT ((type_t)2)
#define TYPE_FLOAT ((type_t)3)

void debug_type(type_t type, ast_debugger_t* debugger);

typedef enum type_name_kind {
    TYPE_NAME_IDENTIFIER,
} type_name_kind_t;

typedef struct type_name {
    type_name_kind_t kind;
    union {
        text_view_t identifier;
    } as;
} type_name_t;

parse_result_t parse_type_name(parser_t* parser, type_name_t* dst);

typedef struct named_type {
    type_t type;
    type_name_t name;
} named_type_t;

#define NAMED_TYPE_UNRESOLVED(name_) \
    (named_type_t){ .name = name_, .type = TYPE_UNRESOLVED}

void debug_named_type(named_type_t type, ast_debugger_t* debugger);
