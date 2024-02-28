#include "ast/expression.h"

void debug_unary_operation(unary_operation_t operation, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "unary_operation");
    ast_debug_key(debugger, "operator");
    switch (operation.operator) {
    case OPERATOR_NEGATE: ast_debug_char(debugger, '-'); break;
    case OPERATOR_NOT: ast_debug_char(debugger, '!'); break;
    case OPERATOR_RETURN: ast_debug_string(debugger, "return"); break;
    case OPERATOR_BREAK: ast_debug_string(debugger, "break"); break;
    }
    ast_debug_key(debugger, "operand");
    debug_expression(*operation.operand, debugger);
    ast_debug_end(debugger);
}

void debug_binary_operation(binary_operation_t operation, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "binary_operation");
    ast_debug_key(debugger, "operator");
    switch (operation.operator) {
    case OPERATOR_ADD: ast_debug_char(debugger, '+'); break;
    case OPERATOR_SUBTRACT: ast_debug_char(debugger, '-'); break;
    case OPERATOR_MULTIPLY: ast_debug_char(debugger, '*'); break;
    case OPERATOR_DIVIDE: ast_debug_char(debugger, '/'); break;
    case OPERATOR_REMAINDER: ast_debug_char(debugger, '%'); break;

    case OPERATOR_BITWISE_OR: ast_debug_char(debugger, '|'); break;
    case OPERATOR_BITWISE_AND: ast_debug_char(debugger, '&'); break;
    case OPERATOR_BITWISE_XOR: ast_debug_char(debugger, '^'); break;

    case OPERATOR_LOGICAL_OR: ast_debug_string(debugger, "or"); break;
    case OPERATOR_LOGICAL_AND: ast_debug_string(debugger, "and"); break;

    case OPERATOR_LESS: ast_debug_char(debugger, '<'); break;
    case OPERATOR_LESS_EQUAL: ast_debug_string(debugger, "<="); break;
    case OPERATOR_EQUAL: ast_debug_string(debugger, "=="); break;
    case OPERATOR_NOT_EQUAL: ast_debug_string(debugger, "!="); break;
    case OPERATOR_GREATER: ast_debug_char(debugger, '>'); break;
    case OPERATOR_GREATER_EQUAL: ast_debug_string(debugger, ">="); break;

    case OPERATOR_INDEX: ast_debug_string(debugger, "in[dex]"); break;

    case OPERATOR_ASSIGN: ast_debug_char(debugger, '='); break;
    }
    ast_debug_key(debugger, "lhs");
    debug_expression(*operation.left, debugger);
    ast_debug_key(debugger, "rhs");
    debug_expression(*operation.right, debugger);
    ast_debug_end(debugger);
}

void debug_call(call_t call, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "call");
    ast_debug_key(debugger, "callee");
    debug_expression(*call.callee, debugger);
    ast_debug_key(debugger, "arguments");
    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i < call.arguments.len; i++) {
        debug_expression(call.arguments.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);
    ast_debug_end(debugger);
}

void debug_binding(binding_t binding, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "binding");
    if (binding.mutable) {
        ast_debug_flag(debugger, "mutable");
    }
    ast_debug_key(debugger, "identifier");
    ast_debug_string_view(debugger, STRING_VIEW(binding.identifier));
    ast_debug_key(debugger, "type");
    debug_named_type(binding.type, debugger);
    ast_debug_key(debugger, "value");
    debug_expression(*binding.value, debugger);
    ast_debug_end(debugger);
}

void debug_block(block_t block, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "block");
    ast_debug_key(debugger, "statements");
    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i < block.statements.len; i++) {
        debug_expression(block.statements.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);
    if (block.has_tail) {
        ast_debug_key(debugger, "tail");
        debug_expression(block.tail, debugger);
    }
    ast_debug_end(debugger);
}

void debug_conditional(conditional_t conditional, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "conditional");

    ast_debug_key(debugger, "condition");
    debug_expression(*conditional.condition, debugger);

    ast_debug_key(debugger, "body");
    debug_block(*conditional.body, debugger);

    switch (conditional.else_kind) {
    case CONDITIONAL_ELSE_NONE:
        break;
    case CONDITIONAL_ELSE_BLOCK:
        ast_debug_key(debugger, "else");
        debug_block(*conditional.else_as.block, debugger);
        break;
    case CONDITIONAL_ELSE_CONDITIONAL:
        ast_debug_key(debugger, "else");
        debug_conditional(*conditional.else_as.conditional, debugger);
        break;
    }

    ast_debug_end(debugger);
}

void debug_loop(loop_t loop, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "loop");
    ast_debug_key(debugger, "body");
    debug_block(*loop.body, debugger);
    ast_debug_end(debugger);
}

void debug_while_loop(while_loop_t while_loop, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "while_loop");
    ast_debug_key(debugger, "condition");
    debug_expression(*while_loop.condition, debugger);
    ast_debug_key(debugger, "body");
    debug_block(*while_loop.body, debugger);
    ast_debug_end(debugger);
}

void debug_expression(expression_t expression, ast_debugger_t* debugger) {
    switch (expression.kind) {
    case EXPRESSION_INTEGER:
        ast_debug_int(debugger, expression.as.integer);
        break;
    case EXPRESSION_VARIABLE:
        ast_debug_string_view(debugger, STRING_VIEW(expression.as.variable));
        break;
    case EXPRESSION_BLOCK:
        debug_block(*expression.as.block, debugger);
        break;
    case EXPRESSION_UNARY_OPERATION:
        debug_unary_operation(expression.as.unary_operation,  debugger);
        break;
    case EXPRESSION_BINARY_OPERATION:
        debug_binary_operation(expression.as.binary_operation,  debugger);
        break;
    case EXPRESSION_CALL:
        debug_call(expression.as.call,  debugger);
        break;
    case EXPRESSION_BINDING:
        debug_binding(expression.as.binding,  debugger);
        break;
    case EXPRESSION_CONDITIONAL:
        debug_conditional(expression.as.conditional,  debugger);
        break;
    case EXPRESSION_LOOP:
        debug_loop(expression.as.loop,  debugger);
        break;
    case EXPRESSION_WHILE_LOOP:
        debug_while_loop(expression.as.while_loop,  debugger);
        break;
    }
}
