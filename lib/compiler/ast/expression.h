#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "compiler/ast/parser.h"
#include "compiler/ast/analyzer.h"
#include "compiler/ast/debug.h"
#include "compiler/ast/type.h"
#include "alloc/array.h"

typedef struct Expression Expression;

typedef enum SymbolExpressionKind {
    SYMBOL_EXPRESSION_VARIABLE,
    SYMBOL_EXPRESSION_FUNCTION,
} SymbolExpressionKind;

typedef struct SymbolExpression {
    TextView name;
    SymbolExpressionKind kind;
    union {
        const Variable* variable;
        const Function* function;
    } as;
} SymbolExpression;

typedef enum UnaryOperator {
    OPERATOR_NEGATE,
    OPERATOR_NOT,
    OPERATOR_RETURN,
    OPERATOR_BREAK,
} UnaryOperator;

typedef struct UnaryOperation {
    UnaryOperator operator;
    Expression* operand;
} UnaryOperation;

typedef enum BinaryOperator {
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
} BinaryOperator;

typedef struct BinaryOperation {
    BinaryOperator operator;
    Expression* left;
    Expression* right;
} BinaryOperation;

typedef ArrayBuf(Expression) ExpressionArrayBuf;

typedef struct MemberAccess {
    Expression* container;
    TextView member_name;
    const Field* field;
} MemberAccess;

typedef struct Call {
    Expression* callee;
    ExpressionArrayBuf arguments;
} Call;

typedef struct Binding {
    Variable variable;
    Expression* value;
} Binding;

typedef struct Block Block;

typedef enum ConditionalElseKind {
    CONDITIONAL_ELSE_NONE,
    CONDITIONAL_ELSE_BLOCK,
    CONDITIONAL_ELSE_CONDITIONAL,
} ConditionalElseKind;

typedef struct Conditional {
    Expression* condition;
    Block* body;
    ConditionalElseKind else_kind;
    union {
        Block* block;
        struct Conditional* conditional;
    } else_as;
} Conditional;

typedef struct InfiniteLoop {
    Block* body;
} InfiniteLoop;

typedef struct WhileLoop {
    Expression* condition;
    Block* body;
} WhileLoop;

typedef enum ExpressionKind {
    EXPRESSION_INVALID,
    EXPRESSION_UNIT,
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
} ExpressionKind;

typedef struct Expression {
    ExpressionKind kind;
    union {
        i64 integer;
        SymbolExpression symbol;
        Block* block;
        UnaryOperation unary_operation;
        BinaryOperation binary_operation;
        MemberAccess member_access;
        Call call;
        Binding binding;
        Conditional conditional;
        InfiniteLoop infinite_loop;
        WhileLoop while_loop;
    } as;
    Type type;
} Expression;

typedef struct Block {
    ExpressionArrayBuf statements;
    bool has_tail : 1;
    Expression tail;
    Scope* scope;
} Block;

Result parse_expression(Parser* parser, Expression* dst);
Result parse_block(Parser* parser, Block* dst);

void debug_unary_operation(UnaryOperation operation, AstDebugger* debugger);
void debug_binary_operation(BinaryOperation operation, AstDebugger* debugger);
void debug_call(Call call, AstDebugger* debugger);
void debug_binding(Binding binding, AstDebugger* debugger);
void debug_block(Block block, AstDebugger* debugger);
void debug_conditional(Conditional conditional, AstDebugger* debugger);
void debug_infinite_loop(InfiniteLoop infinite_loop, AstDebugger* debugger);
void debug_while_loop(WhileLoop while_loop, AstDebugger* debugger);
void debug_expression(Expression expression, AstDebugger* debugger);
