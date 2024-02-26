#include "ast/expression.h"

parse_result_t parse_block(parser_t* parser, block_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    if (!match_parser(parser, TOKEN_LEFT_BRACE, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    expression_array_buf_t statements = new_array_buf(expression_t);
    bool has_tail = false;
    expression_t tail = {0};
    bool last_is_block = false;
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        if (match_parser(parser, TOKEN_SEMICOLON, NULL)) {
            if (has_tail) {
                array_buf_push(&statements, tail);
                has_tail = false;
            }
            last_is_block = false;
            continue;
        }

        if (!last_is_block && has_tail) {
            free_array_buf(statements);
            PARSER_ERROR_RESTORE(parser, state);
        }
        if (has_tail) {
            array_buf_push(&statements, tail);
        }

        expression_t expression;
        if (parse_expression(parser, &expression) != PARSE_SUCCESS) {
            free_array_buf(statements);
            PARSER_ERROR_RESTORE(parser, state);
        }
        
        last_is_block = expression.kind == EXPRESSION_BLOCK
            || expression.kind == EXPRESSION_CONDITIONAL
            || expression.kind == EXPRESSION_LOOP
            || expression.kind == EXPRESSION_WHILE_LOOP;

        has_tail = true;
        tail = expression;
    }
    alloc_stack_push(&parser->storage.allocations, statements.data);

    *dst = (block_t){
        .statements = statements,
        .has_tail = has_tail,
        .tail = tail
    };
    return PARSE_SUCCESS;
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

static parse_result_t parse_integer(parser_t* parser, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    // first token is integer when called
    text_view_t text = peek_parser(*parser).text;
    step_parser(parser);
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
            PARSER_ERROR_RESTORE(parser, state);
        }
        int64_t signed_digit = (text.ptr[i] - '0') * signum;
        if (__builtin_add_overflow(value, signed_digit, &value)) {
            PARSER_ERROR_RESTORE(parser, state);
        }
    }

    *dst = (expression_t){ .kind = EXPRESSION_INTEGER, .as.integer = value };
    return PARSE_SUCCESS;
}

static parse_result_t parse_variable(parser_t* parser, expression_t* dst) {
    // when called, first token is an identifier
    text_view_t identifier = peek_parser(*parser).text;
    step_parser(parser);
    *dst = (expression_t){ .kind = EXPRESSION_VARIABLE, .as.variable = identifier };
    return PARSE_SUCCESS;
}

static parse_result_t
parse_call(parser_t* parser, expression_t callee, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "("

    // parse arguments
    expression_array_buf_t arguments = new_array_buf(expression_t);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        expression_t argument;
        if (parse_expression(parser, &argument) != PARSE_SUCCESS) {
            free_array_buf(arguments);
            PARSER_ERROR_RESTORE(parser, state);
        }
        array_buf_push(&arguments, argument);

        if (!match_parser(parser, TOKEN_COMMA, NULL)) {
            if (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
                free_array_buf(arguments);
                PARSER_ERROR_RESTORE(parser, state);
            }
            break;
        }
    }
    alloc_stack_push(&parser->storage.allocations, arguments.data);

    call_t call = {
        .callee = ast_box(&parser->storage, callee),
        .arguments = arguments
    };
    *dst = (expression_t){ .kind = EXPRESSION_CALL, .as.call = call };
    return PARSE_SUCCESS;
}

static parse_result_t
parse_index(parser_t* parser, expression_t indexee, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "["
    expression_t index;
    if (parse_expression(parser, &index) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }
    if (!match_parser(parser, TOKEN_RIGHT_BRACKET, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    binary_operation_t operation = {
        .operator = OPERATOR_INDEX,
        .left = ast_box(&parser->storage, indexee),
        .right = ast_box(&parser->storage, index),
    };
    *dst = (expression_t){
        .kind = EXPRESSION_BINARY_OPERATION,
        .as.binary_operation = operation
    };
    return PARSE_SUCCESS;
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

static parse_result_t parse_expression_precedence(
    parser_t* super_parser,
    precedence_t precedence,
    expression_t* dst
);

static parse_result_t
parse_prefix(parser_t* parser, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    unary_operator_t operator = as_prefix_operator(peek_parser(*parser));
    if (operator == -1) {
        PARSER_ERROR_RESTORE(parser, state);
    }
    step_parser(parser);

    precedence_t precedence = prefix_operator_precedence(operator);
    expression_t operand;
    if (
        parse_expression_precedence(parser, precedence, &operand)
        != PARSE_SUCCESS
    ) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    unary_operation_t operation = {
        .operator = operator,
        .operand = ast_box(&parser->storage, operand),
    };
    *dst = (expression_t) {
        .kind = EXPRESSION_UNARY_OPERATION,
        .as.unary_operation = operation
    };
    return PARSE_SUCCESS;
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

static parse_result_t
parse_infix(parser_t* parser, expression_t lhs, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    binary_operator_t operator = as_infix_operator(peek_parser(*parser));
    step_parser(parser);
    precedence_t precedence = infix_operator_precedence(operator);
    expression_t rhs;
    if (
        parse_expression_precedence(
            parser,
            next_precedence(precedence),
            &rhs
        ) != PARSE_SUCCESS
    ) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    binary_operation_t operation = {
        .operator = operator,
        .left = ast_box(&parser->storage, lhs),
        .right = ast_box(&parser->storage, rhs),
    };
    *dst = (expression_t){
        .kind = EXPRESSION_BINARY_OPERATION,
        .as.binary_operation = operation
    };
    return PARSE_SUCCESS;
}

static parse_result_t
parse_binding(parser_t* parser, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "let"
    
    bool mutable = match_parser(parser, TOKEN_MUT, NULL);

    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    if (!match_parser(parser, TOKEN_COLON, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    type_name_t type_name;
    if (parse_type_name(parser, &type_name)) {
        PARSER_ERROR_RESTORE(parser, state);
    }
    named_type_t type = NAMED_TYPE_UNRESOLVED(type_name);
    
    if (!match_parser(parser, TOKEN_EQUAL, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    expression_t value;
    if (parse_expression(parser, &value) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    binding_t binding = {
        .mutable = mutable,
        .identifier = identifier.text,
        .type = type,
        .value = ast_box(&parser->storage, value),
    };
    *dst = (expression_t){ .kind = EXPRESSION_BINDING, .as.binding = binding };
    return PARSE_SUCCESS;
}

static parse_result_t
parse_conditional(parser_t* parser, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "if" or "elif"

    expression_t condition;
    if (parse_expression(parser, &condition) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    block_t body;
    if (parse_block(parser, &body) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    conditional_t conditional = {
        .else_kind = CONDITIONAL_ELSE_NONE,
        .else_as = {0}
    };
    while (true) {
        switch (peek_parser(*parser).type) {
        case TOKEN_ELSE:
            step_parser(parser);
            block_t else_block;
            if (parse_block(parser, &else_block) != PARSE_SUCCESS) {
                PARSER_ERROR_RESTORE(parser, state);
            }
            conditional.else_kind = CONDITIONAL_ELSE_BLOCK;
            conditional.else_as.block = ast_box(&parser->storage, else_block);
            break;

        case TOKEN_ELIF:
            ;
            expression_t else_conditional;
            if (parse_conditional(parser, &else_conditional) != PARSE_SUCCESS) {
                PARSER_ERROR_RESTORE(parser, state);
            }
            conditional.else_kind = CONDITIONAL_ELSE_CONDITIONAL;
            conditional.else_as.conditional =
                ast_box(&parser->storage, else_conditional.as.conditional);
            break;

        default: goto parse_else_loop_end;
        }
    }
parse_else_loop_end:

    conditional.condition = ast_box(&parser->storage, condition);
    conditional.body = ast_box(&parser->storage, body);

    *dst = (expression_t){
        .kind = EXPRESSION_CONDITIONAL,
        .as.conditional = conditional
    };
    return PARSE_SUCCESS;
}

static parse_result_t
parse_loop(parser_t* parser, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "loop"

    block_t body;
    if (parse_block(parser, &body) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    loop_t loop = { .body = ast_box(&parser->storage, body) };
    *dst = (expression_t){ .kind = EXPRESSION_LOOP, .as.loop = loop };
    return PARSE_SUCCESS;
}

static parse_result_t
parse_while_loop(parser_t* parser, expression_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "while"

    expression_t condition;
    if (parse_expression(parser, &condition) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    block_t body;
    if (parse_block(parser, &body) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    while_loop_t while_loop = {
        .condition = ast_box(&parser->storage, condition),
        .body = ast_box(&parser->storage, body)
    };
    *dst = (expression_t){
        .kind = EXPRESSION_WHILE_LOOP,
        .as.while_loop = while_loop
    };
    return PARSE_SUCCESS;
}

static parse_result_t parse_expression_precedence(
    parser_t* parser,
    precedence_t precedence,
    expression_t* dst
) {
    parser_state_t state = parser_snapshot(*parser);
    parse_result_t error = PARSE_ERROR;
    expression_t expression;

    unary_operator_t unary_operator = as_prefix_operator(peek_parser(*parser));
    if (unary_operator != -1) {
        error = parse_prefix(parser, &expression);
    } else switch (peek_parser(*parser).type) {
    case TOKEN_INTEGER:
        error = parse_integer(parser, &expression);
        break;

    case TOKEN_IDENTIFIER:
        error = parse_variable(parser, &expression);
        break;

    case TOKEN_LEFT_PAREN:
        step_parser(parser);
        if (parse_expression(parser, &expression) != PARSE_SUCCESS) {
            PARSER_ERROR_RESTORE(parser, state);
        }
        if (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
            PARSER_ERROR_RESTORE(parser, state);
        }
        break;

    case TOKEN_LEFT_BRACE:
        ;
        block_t block;
        if (parse_block(parser, &block) != PARSE_SUCCESS) {
            PARSER_ERROR_RESTORE(parser, state);
        }
        error = PARSE_SUCCESS;
        expression = (expression_t){
            .kind = EXPRESSION_BLOCK,
            .as.block = ast_box(&parser->storage, block)
        };
        break;

    case TOKEN_LET:
        error = parse_binding(parser, &expression);
        break;

    case TOKEN_IF:
        error = parse_conditional(parser, &expression);
        break;

    case TOKEN_LOOP:
        error = parse_loop(parser, &expression);
        break;

    case TOKEN_WHILE:
        error = parse_while_loop(parser, &expression);
        break;

    default: break;
    }

    if (error != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    while (true) {
        switch (peek_parser(*parser).type) {
        case TOKEN_LEFT_PAREN:
            error = parse_call(parser, expression, &expression);
            break;
        case TOKEN_LEFT_BRACKET:
            error = parse_index(parser, expression, &expression);
            break;
        default:
            goto parse_call_or_index_loop_end;
        }

        if (error != PARSE_SUCCESS) {
            PARSER_ERROR_RESTORE(parser, state);
        }
    }
parse_call_or_index_loop_end:

    while (true) {
        binary_operator_t binary_operator = as_infix_operator(peek_parser(*parser));
        if (binary_operator == -1) {
            break;
        }
        precedence_t operator_precedence = infix_operator_precedence(binary_operator);

        if (!precedence_compatible(precedence, operator_precedence)) {
            PARSER_ERROR_RESTORE(parser, state);
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

        error = parse_infix(parser, expression, &expression);
        if (error != PARSE_SUCCESS) {
            PARSER_ERROR_RESTORE(parser, state);
        }
    }

    *dst = expression;
    return PARSE_SUCCESS;
}

parse_result_t parse_expression(parser_t* parser, expression_t* dst) {
    return parse_expression_precedence(parser, PRECEDENCE_NONE, dst);
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
