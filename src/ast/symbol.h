#pragma once

#include "text/text.h"
#include "ast/function.h"
#include "ast/type.h"

typedef enum symbol_kind {
    SYMBOL_TYPE,
    SYMBOL_FUNCTION,
    SYMBOL_VARIABLE,
} symbol_kind_t;

typedef struct symbol {
    text_view_t name;
    symbol_kind_t kind;
    union {
        element_type_t* type;
        function_t* function;
        variable_t variable;
    } as;
} symbol_t;

typedef array_buf_t(symbol_t) symbol_array_buf_t;
