#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ast/parser.h"
#include "ast/debug.h"
#include "ast/type.h"
#include "util/array.h"

typedef struct expression expression_t;

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

typedef struct call {
    expression_t* callee;
    expression_array_buf_t arguments;
} call_t;

typedef struct binding {
    bool mutable;
    text_view_t identifier;
    named_type_t type;
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

typedef struct loop {
    block_t* body;
} loop_t;

typedef struct while_loop {
    expression_t* condition;
    block_t* body;
} while_loop_t;

typedef enum expression_kind {
    EXPRESSION_INTEGER,
    EXPRESSION_VARIABLE,
    EXPRESSION_BLOCK,
    EXPRESSION_UNARY_OPERATION,
    EXPRESSION_BINARY_OPERATION,
    EXPRESSION_CALL,
    EXPRESSION_BINDING,
    EXPRESSION_CONDITIONAL,
    EXPRESSION_LOOP,
    EXPRESSION_WHILE_LOOP,
    // EXPRESSION_FOR_LOOP,
} expression_kind_t;

typedef struct expression {
    expression_kind_t kind;
    union {
        int64_t integer;
        text_view_t variable;
        block_t* block;
        unary_operation_t unary_operation;
        binary_operation_t binary_operation;
        call_t call;
        binding_t binding;
        conditional_t conditional;
        loop_t loop;
        while_loop_t while_loop;
    } as;
} expression_t;

parse_result_t parse_expression(parser_t* parser, expression_t* dst);

typedef struct block {
    expression_array_buf_t statements;
    bool has_tail : 1;
    expression_t tail;
} block_t;

parse_result_t parse_block(parser_t* parser, block_t* dst);

void debug_unary_operation(unary_operation_t operation, ast_debugger_t* debugger);
void debug_binary_operation(binary_operation_t operation, ast_debugger_t* debugger);
void debug_call(call_t call, ast_debugger_t* debugger);
void debug_binding(binding_t binding, ast_debugger_t* debugger);
void debug_block(block_t block, ast_debugger_t* debugger);
void debug_conditional(conditional_t conditional, ast_debugger_t* debugger);
void debug_loop(loop_t loop, ast_debugger_t* debugger);
void debug_while_loop(while_loop_t while_loop, ast_debugger_t* debugger);
void debug_expression(expression_t expression, ast_debugger_t* debugger);
