#include "ast/expression.h"

parse_block_result_t parse_block(parser_t* super_parser) {
    parser_t parser = *super_parser;
    if (!match_parser(&parser, TOKEN_LEFT_BRACE, NULL)) {
        return RESULT_ERROR(parse_block_result_t, PARSE_ERROR);
    }

    block_t block = { .statements = new_array_buf(), .has_tail = false };
    bool last_is_block = false;
    while (!match_parser(&parser, TOKEN_RIGHT_BRACE, NULL)) {
        if (match_parser(&parser, TOKEN_SEMICOLON, NULL)) {
            if (block.has_tail) {
                array_buf_push(&block.statements, &block.tail, sizeof(block.tail));
                block.has_tail = false;
            }
            last_is_block = false;
            continue;
        }

        if (!last_is_block && block.has_tail) {
            return RESULT_ERROR(parse_block_result_t, PARSE_ERROR);
        }
        if (block.has_tail) {
            array_buf_push(&block.statements, &block.tail, sizeof(block.tail));
        }

        parse_expression_result_t expression = parse_expression(&parser);
        if (!expression.is_ok) {
            destroy_array_buf(block.statements);
            return CAST_ERROR(parse_block_result_t, expression);
        }
        
        last_is_block = expression.ok.kind == EXPRESSION_BLOCK
            || expression.ok.kind == EXPRESSION_CONDITIONAL
            || expression.ok.kind == EXPRESSION_LOOP
            || expression.ok.kind == EXPRESSION_WHILE_LOOP;

        block.has_tail = true;
        block.tail = expression.ok;
    }

    *super_parser = parser;

    return RESULT_OK(parse_block_result_t, block);
}

// orders of operations:
// - primary -> unary -> multiplication -> addition -> comparison -> logical
// - primary -> unary -> bitwise -> comparison -> logical
typedef enum precedence {
    PRECEDENCE_NONE,
    PRECEDENCE_DIVERGE,
    PRECEDENCE_ASSIGN,
    PRECEDENCE_LOGICAL,
    PRECEDENCE_COMPARISON,
    PRECEDENCE_BITWISE_OR_ADDITION,
    PRECEDENCE_BITWISE,
    PRECEDENCE_ADDITION,
    PRECEDENCE_MULTIPLICATION,
    PRECEDENCE_UNARY,
    PRECEDENCE_PRIMARY,
} precedence_t;

static bool precedence_compatible(precedence_t lhs, precedence_t rhs) {
    if (lhs == PRECEDENCE_COMPARISON && rhs == PRECEDENCE_COMPARISON) {
        return false;
    }
    if (lhs == PRECEDENCE_BITWISE
        && (rhs == PRECEDENCE_ADDITION || rhs == PRECEDENCE_MULTIPLICATION)) {
        return false;
    }
    if (rhs == PRECEDENCE_BITWISE
        && (lhs == PRECEDENCE_ADDITION || lhs == PRECEDENCE_MULTIPLICATION)) {
        return false;
    }
    return true;
}

static precedence_t next_precedence(precedence_t precedence) {
    if (precedence == PRECEDENCE_BITWISE) {
        return PRECEDENCE_UNARY;
    }
    return precedence + 1;
}

static parse_expression_result_t parse_integer(parser_t* super_parser) {
    parser_t parser = *super_parser;
    text_view_t text = peek_parser(parser).text;
    step_parser(&parser);
    size_t i = 0;

    int64_t signum;
    switch (text.ptr[i]) {
    case '-':
        signum = -1;
        i++;
        break;
    case '+':
        signum = 1;
        i++;
        break;
    default:
        signum = 1;
        break;
    }

    int64_t value = 0;
    for (; i < text.len; i++) {
        if (__builtin_mul_overflow(value, 10, &value)) {
            return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }
        int64_t signed_digit = (text.ptr[i] - '0') * signum;
        if (__builtin_add_overflow(value, signed_digit, &value)) {
                return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }
    }

    *super_parser = parser;
    expression_t expression = { .kind = EXPRESSION_INTEGER, .as.integer = value };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t parse_variable(parser_t* parser) {
    // when called, first token is an identifier
    text_view_t identifier = peek_parser(*parser).text;
    step_parser(parser);
    expression_t expression = { .kind = EXPRESSION_VARIABLE, .as.variable = identifier };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t parse_call(parser_t* super_parser, expression_t callee) {
    parser_t parser = *super_parser;
    step_parser(&parser); // skip "("

    // parse arguments.
    array_buf_t arguments = new_array_buf();
    while (!match_parser(&parser, TOKEN_RIGHT_PAREN, NULL)) {
        parse_expression_result_t argument = parse_expression(&parser);
        if (!argument.is_ok) {
            return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }
        array_buf_push(&arguments, &argument.ok, sizeof(argument.ok));

        if (!match_parser(&parser, TOKEN_COMMA, NULL)) {
            if (!match_parser(&parser, TOKEN_RIGHT_PAREN, NULL)) {
                return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
            }
            break;
        }
    }

    *super_parser = parser;
    call_t call = { .callee = malloc(sizeof(callee)), .arguments = arguments };
    *call.callee = callee;
    expression_t expression = { .kind = EXPRESSION_CALL, .as.call = call };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t parse_index(parser_t* super_parser, expression_t indexee) {
    parser_t parser = *super_parser;
    step_parser(&parser); // skip "["
    parse_expression_result_t index = parse_expression(&parser);
    if (!index.is_ok) {
        return index;
    }
    if (!match_parser(&parser, TOKEN_RIGHT_BRACKET, NULL)) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }

    *super_parser = parser;
    binary_operation_t operation = {
        .operator = OPERATOR_INDEX,
        .left = malloc(sizeof(indexee)),
        .right = malloc(sizeof(index.ok)),
    };
    *operation.left = indexee;
    *operation.right = index.ok;
    expression_t expression = {
        .kind = EXPRESSION_BINARY_OPERATION,
        .as.binary_operation = operation
    };
    return RESULT_OK(parse_expression_result_t, expression);
}

static unary_operator_t as_prefix_operator(token_t token) {
    switch (token.type) {
    case TOKEN_MINUS: return OPERATOR_NEGATE;
    case TOKEN_BANG: return OPERATOR_NOT;
    case TOKEN_RETURN: return OPERATOR_RETURN;
    case TOKEN_BREAK: return OPERATOR_BREAK;
    default: return -1;
    }
}

static precedence_t prefix_operator_precedence(unary_operator_t operator) {
    switch (operator) {
    case OPERATOR_NOT:
    case OPERATOR_NEGATE:
        return PRECEDENCE_UNARY;
    case OPERATOR_RETURN:
    case OPERATOR_BREAK:
        return PRECEDENCE_DIVERGE;
    }
}

static parse_expression_result_t
parse_expression_precedence(parser_t* super_parser, precedence_t precendence);

static parse_expression_result_t
parse_prefix(parser_t* super_parser) {
    parser_t parser = *super_parser;

    unary_operation_t operation;

    operation.operator = as_prefix_operator(peek_parser(parser));
    if (operation.operator == -1) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }
    step_parser(&parser);

    precedence_t precedence = prefix_operator_precedence(operation.operator);
    parse_expression_result_t operand =
        parse_expression_precedence(&parser, precedence);
    if (!operand.is_ok) {
        return operand;
    }
    operation.operand = malloc(sizeof(expression_t));
    *operation.operand = operand.ok;

    *super_parser = parser;
    expression_t expression = {
        .kind = EXPRESSION_UNARY_OPERATION,
        .as.unary_operation = operation
    };
    return RESULT_OK(parse_expression_result_t, expression);
}

static binary_operator_t as_infix_operator(token_t token) {
    switch (token.type) {
    case TOKEN_PLUS: return OPERATOR_ADD;
    case TOKEN_MINUS: return OPERATOR_SUBTRACT;
    case TOKEN_STAR: return OPERATOR_MULTIPLY;
    case TOKEN_SLASH: return OPERATOR_DIVIDE;
    case TOKEN_PERCENT: return OPERATOR_REMAINDER;

    case TOKEN_TUBE: return OPERATOR_BITWISE_OR;
    case TOKEN_AMPERSAND: return OPERATOR_BITWISE_AND;
    case TOKEN_CARET: return OPERATOR_BITWISE_XOR;

    case TOKEN_OR: return OPERATOR_LOGICAL_OR;
    case TOKEN_AND: return OPERATOR_LOGICAL_AND;
    
    case TOKEN_LESS: return OPERATOR_LESS;
    case TOKEN_LESS_EQUAL: return OPERATOR_LESS_EQUAL;
    case TOKEN_EQUAL_EQUAL: return OPERATOR_EQUAL;
    case TOKEN_BANG_EQUAL: return OPERATOR_NOT_EQUAL;
    case TOKEN_GREATER: return OPERATOR_GREATER;
    case TOKEN_GREATER_EQUAL: return OPERATOR_GREATER_EQUAL;

    case TOKEN_EQUAL: return OPERATOR_ASSIGN;

    default: return -1;
    }
}

static precedence_t infix_operator_precedence(binary_operator_t operator) {
    switch (operator) {
    case OPERATOR_ADD:
    case OPERATOR_SUBTRACT:
        return PRECEDENCE_ADDITION;

    case OPERATOR_MULTIPLY:
    case OPERATOR_DIVIDE:
    case OPERATOR_REMAINDER:
        return PRECEDENCE_MULTIPLICATION;

    case OPERATOR_BITWISE_OR:
    case OPERATOR_BITWISE_AND:
    case OPERATOR_BITWISE_XOR:
        return PRECEDENCE_BITWISE;

    case OPERATOR_LOGICAL_OR:
    case OPERATOR_LOGICAL_AND:
        return PRECEDENCE_LOGICAL;

    case OPERATOR_LESS:
    case OPERATOR_LESS_EQUAL:
    case OPERATOR_EQUAL:
    case OPERATOR_NOT_EQUAL:
    case OPERATOR_GREATER:
    case OPERATOR_GREATER_EQUAL:
        return PRECEDENCE_COMPARISON;

    case OPERATOR_ASSIGN:
        return PRECEDENCE_ASSIGN;

    default: return -1;
    }
}

static parse_expression_result_t
parse_infix(parser_t* super_parser, expression_t lhs) {
    parser_t parser = *super_parser;
    binary_operator_t operator = as_infix_operator(peek_parser(parser));
    step_parser(&parser);
    precedence_t precedence = infix_operator_precedence(operator);
    parse_expression_result_t rhs =
        parse_expression_precedence(&parser, next_precedence(precedence));

    if (!rhs.is_ok) {
        return rhs;
    }

    *super_parser = parser;

    binary_operation_t operation = {
        .left = malloc(sizeof(lhs)),
        .operator = operator,
        .right = malloc(sizeof(rhs.ok)),
    };
    *operation.left = lhs;
    *operation.right = rhs.ok;
    expression_t expression = {
        .kind = EXPRESSION_BINARY_OPERATION,
        .as.binary_operation = operation
    };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t
parse_binding(parser_t* super_parser) {
    parser_t parser = *super_parser;

    step_parser(&parser); // skip "let"
    
    bool mutable = match_parser(&parser, TOKEN_MUT, NULL);

    token_t identifier;
    if (!match_parser(&parser, TOKEN_IDENTIFIER, &identifier)) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }

    if (!match_parser(&parser, TOKEN_COLON, NULL)) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }
    parse_type_name_result_t type_name = parse_type_name(&parser);
    if (!type_name.is_ok) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }
    named_type_t type = NAMED_TYPE_UNRESOLVED(type_name.ok);
    
    if (!match_parser(&parser, TOKEN_EQUAL, NULL)) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }

    parse_expression_result_t value = parse_expression(&parser);
    if (!value.is_ok) {
        return value;
    }

    *super_parser = parser;

    binding_t binding = {
        .mutable = mutable,
        .identifier = identifier.text,
        .type = type,
        .value = malloc(sizeof(value.ok)),
    };
    *binding.value = value.ok;
    expression_t expression = { .kind = EXPRESSION_BINDING, .as.binding = binding };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t
parse_conditional(parser_t* super_parser) {
    parser_t parser = *super_parser;
    step_parser(&parser); // skip "if" or "elif"

    parse_expression_result_t condition = parse_expression(&parser);
    if (!condition.is_ok) {
        return condition;
    }

    parse_block_result_t body = parse_block(&parser);
    if (!body.is_ok) {
        return CAST_ERROR(parse_expression_result_t, body);
    }

    conditional_t conditional = {
        .condition = NULL, // filled out later
        .body = NULL, // filled out later
        .else_kind = CONDITIONAL_ELSE_NONE,
        .else_as = {0}
    };
    while (true) {
        switch (peek_parser(parser).type) {
        case TOKEN_ELSE:
            step_parser(&parser);
            parse_block_result_t else_block = parse_block(&parser);
            if (!else_block.is_ok) {
                return CAST_ERROR(parse_expression_result_t, else_block);
            }
            conditional.else_kind = CONDITIONAL_ELSE_BLOCK;
            conditional.else_as.block = malloc(sizeof(block_t));
            *conditional.else_as.block = else_block.ok;
            break;

        case TOKEN_ELIF:
            ;
            parse_expression_result_t else_conditional = parse_conditional(&parser);
            if (!else_conditional.is_ok) {
                return else_conditional;
            }
            conditional.else_kind = CONDITIONAL_ELSE_CONDITIONAL;
            conditional.else_as.conditional = malloc(sizeof(conditional_t));
            *conditional.else_as.conditional = else_conditional.ok.as.conditional;
            break;

        default: goto parse_else_loop_end;
        }
    }
parse_else_loop_end:

    conditional.condition = malloc(sizeof(condition.ok));
    *conditional.condition = condition.ok;
    conditional.body = malloc(sizeof(body.ok));
    *conditional.body = body.ok;

    *super_parser = parser;
    expression_t expression = {
        .kind = EXPRESSION_CONDITIONAL,
        .as.conditional = conditional
    };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t
parse_loop(parser_t* super_parser) {
    parser_t parser = *super_parser;
    step_parser(&parser); // skip "loop"

    parse_block_result_t body = parse_block(&parser);
    if (!body.is_ok) {
        return CAST_ERROR(parse_expression_result_t, body);
    }

    *super_parser = parser;
    loop_t loop = { .body = malloc(sizeof(body.ok)) };
    *loop.body = body.ok;
    expression_t expression = { .kind = EXPRESSION_LOOP, .as.loop = loop };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t
parse_while_loop(parser_t* super_parser) {
    parser_t parser = *super_parser;
    step_parser(&parser); // skip "while"

    parse_expression_result_t condition = parse_expression(&parser);
    if (!condition.is_ok) {
        return condition;
    }

    parse_block_result_t body = parse_block(&parser);
    if (!body.is_ok) {
        return CAST_ERROR(parse_expression_result_t, body);
    }

    *super_parser = parser;
    while_loop_t while_loop = {
        .condition = malloc(sizeof(condition.ok)),
        .body = malloc(sizeof(body.ok))
    };
    *while_loop.condition = condition.ok;
    *while_loop.body = body.ok;
    expression_t expression = {
        .kind = EXPRESSION_WHILE_LOOP,
        .as.while_loop = while_loop
    };
    return RESULT_OK(parse_expression_result_t, expression);
}

static parse_expression_result_t
parse_expression_precedence(parser_t* super_parser, precedence_t precedence) {
    parser_t parser = *super_parser;
    parse_expression_result_t expression =
        RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);

    unary_operator_t unary_operator = as_prefix_operator(peek_parser(parser));
    if (unary_operator != -1) {
        expression = parse_prefix(&parser);
    } else switch (peek_parser(parser).type) {
    case TOKEN_INTEGER:
        expression = parse_integer(&parser);
        break;

    case TOKEN_IDENTIFIER:
        expression = parse_variable(&parser);
        break;

    case TOKEN_LEFT_PAREN:
        step_parser(&parser);
        expression = parse_expression(&parser);
        if (!expression.is_ok) {
            return expression;
        }
        if (!match_parser(&parser, TOKEN_RIGHT_PAREN, NULL)) {
            return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }
        break;

    case TOKEN_LEFT_BRACE:
        ;
        parse_block_result_t block = parse_block(&parser);
        if (!block.is_ok) {
            return CAST_ERROR(parse_expression_result_t, block);
        }
        expression_t block_expression = {
            .kind = EXPRESSION_BLOCK,
            .as.block = malloc(sizeof(block_t)),
        };
        *block_expression.as.block = block.ok;
        expression = RESULT_OK(parse_expression_result_t, block_expression);
        break;

    case TOKEN_LET:
        expression = parse_binding(&parser);
        break;

    case TOKEN_IF:
        expression = parse_conditional(&parser);
        break;

    case TOKEN_LOOP:
        expression = parse_loop(&parser);
        break;

    case TOKEN_WHILE:
        expression = parse_while_loop(&parser);
        break;

    default: break;
    }

    if (!expression.is_ok) {
        return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
    }

    while (true) {
        switch (peek_parser(parser).type) {
        case TOKEN_LEFT_PAREN:
            expression = parse_call(&parser, expression.ok);
            break;
        case TOKEN_LEFT_BRACKET:
            expression = parse_index(&parser, expression.ok);
            break;
        default:
            goto parse_call_or_index_loop_end;
        }

        if (!expression.is_ok) {
            return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }
    }
parse_call_or_index_loop_end:

    while (true) {
        binary_operator_t binary_operator = as_infix_operator(peek_parser(parser));
        if (binary_operator == -1) {
            break;
        }
        precedence_t operator_precedence = infix_operator_precedence(binary_operator);

        if (!precedence_compatible(precedence, operator_precedence)) {
            return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }

        if (precedence == PRECEDENCE_BITWISE_OR_ADDITION) {
            if (operator_precedence == PRECEDENCE_BITWISE
                || operator_precedence == PRECEDENCE_ADDITION
            ) {
                precedence = operator_precedence;
            }
        }

        if (operator_precedence < precedence) {
            break;
        }

        expression = parse_infix(&parser, expression.ok);
        if (!expression.is_ok) {
            return RESULT_ERROR(parse_expression_result_t, PARSE_ERROR);
        }
    }

    *super_parser = parser;
    return expression;
}

parse_expression_result_t parse_expression(parser_t* parser) {
    return parse_expression_precedence(parser, PRECEDENCE_NONE);
}

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
    for (size_t i = 0; i * sizeof(expression_t) < call.arguments.len; i++) {
        debug_expression(((expression_t*)(call.arguments.ptr))[i], debugger);
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
    for (size_t i = 0; i * sizeof(expression_t) < block.statements.len; i++) {
        debug_expression(((expression_t*)block.statements.ptr)[i], debugger);
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
        debug_unary_operation(expression.as.unary_operation, debugger);
        break;
    case EXPRESSION_BINARY_OPERATION:
        debug_binary_operation(expression.as.binary_operation, debugger);
        break;
    case EXPRESSION_CALL:
        debug_call(expression.as.call, debugger);
        break;
    case EXPRESSION_BINDING:
        debug_binding(expression.as.binding, debugger);
        break;
    case EXPRESSION_CONDITIONAL:
        debug_conditional(expression.as.conditional, debugger);
        break;
    case EXPRESSION_LOOP:
        debug_loop(expression.as.loop, debugger);
        break;
    case EXPRESSION_WHILE_LOOP:
        debug_while_loop(expression.as.while_loop, debugger);
        break;
    }
}
