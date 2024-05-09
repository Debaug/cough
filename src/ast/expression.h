#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ast/parser.h"
#include "ast/analyzer.h"
#include "ast/debug.h"
#include "ast/type.h"
#include "alloc/array.h"

typedef struct expression expression_t;

typedef enum symbol_expression_kind {
    SYMBOL_EXPRESSION_VARIABLE,
    SYMBOL_EXPRESSION_FUNCTION,
} symbol_expression_kind_t;

typedef struct symbol_expression {
    text_view_t name;
    symbol_expression_kind_t kind;
    union {
        const variable_t* variable;
        const function_t* function;
    } as;
} symbol_expression_t;

typedef enum unary_operator {
    OPERATOR_NEGATE,
    OPERATOR_NOT,
    OPERATOR_RETURN,
    OPERATOR_BREAK,
} unary_operator_t;

typedef struct unary_operation {
    unary_operator_t operator;
    expression_t* operand;
} unary_operation_t;

typedef enum binary_operator {
    OPERATOR_ADD,
    OPERATOR_SUBTRACT,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_REMAINDER,

    OPERATOR_BITWISE_OR,
    OPERATOR_BITWISE_AND,
    OPERATOR_BITWISE_XOR,

    OPERATOR_LOGICAL_OR,
    OPERATOR_LOGICAL_AND,

    OPERATOR_LESS,
    OPERATOR_LESS_EQUAL,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_GREATER,
    OPERATOR_GREATER_EQUAL,

    OPERATOR_INDEX,

    OPERATOR_ASSIGN,
} binary_operator_t;

typedef struct binary_operation {
    binary_operator_t operator;
    expression_t* left;
    expression_t* right;
} binary_operation_t;

typedef array_buf_t(expression_t) expression_array_buf_t;

typedef struct member_access {
    expression_t* container;
    text_view_t member_name;
    const field_t* field;
} member_access_t;

typedef struct call {
    expression_t* callee;
    expression_array_buf_t arguments;
} call_t;

typedef struct binding {
    variable_t variable;
    expression_t* value;
} binding_t;

typedef struct block block_t;

typedef enum conditional_else_kind {
    CONDITIONAL_ELSE_NONE,
    CONDITIONAL_ELSE_BLOCK,
    CONDITIONAL_ELSE_CONDITIONAL,
} conditional_else_kind_t;

typedef struct conditional {
    expression_t* condition;
    block_t* body;
    conditional_else_kind_t else_kind;
    union {
        block_t* block;
        struct conditional* conditional;
    } else_as;
} conditional_t;

typedef struct infinite_loop {
    block_t* body;
} infinite_loop_t;

typedef struct while_loop {
    expression_t* condition;
    block_t* body;
} while_loop_t;

typedef enum expression_kind {
    EXPRESSION_INTEGER,
    EXPRESSION_SYMBOL,
    EXPRESSION_BLOCK,
    EXPRESSION_UNARY_OPERATION,
    EXPRESSION_BINARY_OPERATION,
    EXPRESSION_MEMBER_ACCESS,
    EXPRESSION_CALL,
    EXPRESSION_BINDING,
    EXPRESSION_CONDITIONAL,
    EXPRESSION_INFINITE_LOOP,
    EXPRESSION_WHILE_LOOP,
    // EXPRESSION_FOR_LOOP,
} expression_kind_t;

typedef struct expression {
    expression_kind_t kind;
    union {
        int64_t integer;
        symbol_expression_t symbol;
        block_t* block;
        unary_operation_t unary_operation;
        binary_operation_t binary_operation;
        member_access_t member_access;
        call_t call;
        binding_t binding;
        conditional_t conditional;
        infinite_loop_t infinite_loop;
        while_loop_t while_loop;
    } as;
    type_t type;
} expression_t;

typedef struct block {
    expression_array_buf_t statements;
    bool has_tail : 1;
    expression_t tail;
    scope_t* scope;
} block_t;

parse_result_t parse_expression(parser_t* parser, expression_t* dst);
parse_result_t parse_block(parser_t* parser, block_t* dst);

analyze_result_t analyze_expression(
    analyzer_t* analyzer,
    expression_t* expression,
    type_t* return_type,
    type_t* break_type
);
analyze_result_t analyze_block(
    analyzer_t* analyzer,
    block_t* block,
    type_t* return_type,
    type_t* break_type
);

void debug_unary_operation(unary_operation_t operation, ast_debugger_t* debugger);
void debug_binary_operation(binary_operation_t operation, ast_debugger_t* debugger);
void debug_call(call_t call, ast_debugger_t* debugger);
void debug_binding(binding_t binding, ast_debugger_t* debugger);
void debug_block(block_t block, ast_debugger_t* debugger);
void debug_conditional(conditional_t conditional, ast_debugger_t* debugger);
void debug_infinite_loop(infinite_loop_t infinite_loop, ast_debugger_t* debugger);
void debug_while_loop(while_loop_t while_loop, ast_debugger_t* debugger);
void debug_expression(expression_t expression, ast_debugger_t* debugger);
