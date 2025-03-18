#include "ast/expression.h"

void debug_unary_operation(UnaryOperation operation, AstDebugger* debugger) {
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

void debug_binary_operation(BinaryOperation operation, AstDebugger* debugger) {
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

void debug_member_access(MemberAccess member_access, AstDebugger* debugger) {
    ast_debug_start(debugger, "member_access");
    ast_debug_key(debugger, "container");
    debug_expression(*member_access.container, debugger);
    ast_debug_key(debugger, "field");
    if (member_access.field != NULL) {
        debug_field(*member_access.field, debugger);
    } else {
        ast_debug_string_view(debugger, STRING_VIEW(member_access.member_name));
    }
    ast_debug_end(debugger);
}

void debug_call(Call call, AstDebugger* debugger) {
    ast_debug_start(debugger, "call");
    ast_debug_key(debugger, "callee");
    debug_expression(*call.callee, debugger);
    ast_debug_key(debugger, "arguments");
    ast_debug_start_sequence(debugger);
    for (usize i = 0; i < call.arguments.len; i++) {
        debug_expression(call.arguments.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);
    ast_debug_end(debugger);
}

void debug_binding(Binding binding, AstDebugger* debugger) {
    ast_debug_start(debugger, "binding");
    ast_debug_key(debugger, "variable");
    debug_variable(binding.variable, debugger);
    ast_debug_key(debugger, "value");
    debug_expression(*binding.value, debugger);
    ast_debug_end(debugger);
}

void debug_block(Block block, AstDebugger* debugger) {
    ast_debug_start(debugger, "block");
    ast_debug_key(debugger, "statements");
    ast_debug_start_sequence(debugger);
    for (usize i = 0; i < block.statements.len; i++) {
        debug_expression(block.statements.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);
    if (block.has_tail) {
        ast_debug_key(debugger, "tail");
        debug_expression(block.tail, debugger);
    }
    ast_debug_end(debugger);
}

void debug_conditional(Conditional conditional, AstDebugger* debugger) {
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

void debug_infinite_loop(InfiniteLoop infinite_loop, AstDebugger* debugger) {
    ast_debug_start(debugger, "infinite_loop");
    ast_debug_key(debugger, "body");
    debug_block(*infinite_loop.body, debugger);
    ast_debug_end(debugger);
}

void debug_while_loop(WhileLoop while_loop, AstDebugger* debugger) {
    ast_debug_start(debugger, "while_loop");
    ast_debug_key(debugger, "condition");
    debug_expression(*while_loop.condition, debugger);
    ast_debug_key(debugger, "body");
    debug_block(*while_loop.body, debugger);
    ast_debug_end(debugger);
}

void debug_expression(Expression expression, AstDebugger* debugger) {
    switch (expression.kind) {
    case EXPRESSION_INVALID:
        ast_debug_string(debugger, "<invalid>");
        break;
    case EXPRESSION_UNIT:
        ast_debug_string(debugger, "<unit>");
    case EXPRESSION_INTEGER:
        ast_debug_int(debugger, expression.as.integer);
        break;
    case EXPRESSION_SYMBOL:
        ast_debug_string_view(debugger, STRING_VIEW(expression.as.symbol.name));
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
    case EXPRESSION_MEMBER_ACCESS:
        debug_member_access(expression.as.member_access, debugger);
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
    case EXPRESSION_INFINITE_LOOP:
        debug_infinite_loop(expression.as.infinite_loop,  debugger);
        break;
    case EXPRESSION_WHILE_LOOP:
        debug_while_loop(expression.as.while_loop,  debugger);
        break;
    }
}
