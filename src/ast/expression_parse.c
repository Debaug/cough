#include "ast/expression.h"

typedef enum parse_delimited_result {
    DELIMITED_SUCCESS,
    DELIMITED_UNOPENED,
    DELIMITED_EMPTY,
    DELIMITED_INVALID_INSIDE,
    DELIMITED_EXCESS_INSIDE,
    DELIMITED_UNCLOSED,
} parse_delimited_result_t;

static parse_delimited_result_t parse_delimited_expression(
    parser_t* parser,
    token_type_t opening,
    token_type_t closing,
    expression_t* dst,
    token_t* opening_token,
    token_t* invalid_closing_token
) {
    if (!match_parser(parser, opening, opening_token)) {
        return DELIMITED_UNOPENED;
    }
    if (match_parser(parser, closing, NULL)) {
        return DELIMITED_EMPTY;
    }

    expression_t inside;
    parser_alloc_state_t state = parser_snapshot(*parser);
    parse_delimited_result_t result;
    if (parse_expression(parser, &inside) != SUCCESS) {
        result = DELIMITED_INVALID_INSIDE;
    } else if (!match_parser(parser, closing, NULL)) {
        *invalid_closing_token = peek_parser(*parser);
        result = DELIMITED_EXCESS_INSIDE;
    } else {
        result = DELIMITED_SUCCESS;
    }

    if (result == DELIMITED_INVALID_INSIDE || result == DELIMITED_EXCESS_INSIDE) {
        parser_restore(parser, state);
        skip_parser_until(parser, closing);
        if (parser_is_eof(*parser)) {
            return DELIMITED_UNCLOSED;
        }
        step_parser(parser);
        return result;
    }

    *dst = inside;
    return DELIMITED_SUCCESS;
}

static void skip_parser_invalid_statement(parser_t* parser) {
    token_type_t skip_until[2] = { TOKEN_RIGHT_BRACE, TOKEN_SEMICOLON };
    skip_parser_until_any_of(parser, skip_until, 2);
}

result_t parse_block(parser_t* parser, block_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);

    token_t opening_brace;
    if (!match_parser(parser, TOKEN_LEFT_BRACE, &opening_brace)) {
        error_t error = {
            .kind = ERROR_EXPECTED_BLOCK,
            .source = peek_parser(*parser).text,
            .message = format("expected block (delimited by curly braces `{`, `}`)"),
        };
        parser_error_restore(parser, state, error);
        return ERROR;
    }

    expression_array_buf_t statements = new_array_buf(expression_t);
    bool has_tail = false;
    expression_t tail = {0};
    bool last_is_block = false;
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        if (parser_is_eof(*parser)) {
            error_t error = {
                .kind = ERROR_UNCLOSED_BRACES,
                .source = opening_brace.text,
                .message = format("unclosed block (delimited by curly braces `{` `}`)")
            };
            parser_error_restore(parser, state, error);
            return ERROR;
        }

        if (match_parser(parser, TOKEN_SEMICOLON, NULL)) {
            if (has_tail) {
                array_buf_push(&statements, tail);
                has_tail = false;
            }
            last_is_block = false;
            continue;
        }

        if (!last_is_block && has_tail) {
            error_t error = {
                .kind = ERROR_EXPECTED_BLOCK_END,
                .source = peek_parser(*parser).text,
                .message = format("statements must be separated by a semicolon `;`")
            };
            report(parser->reporter, error);
            continue;
        }
        if (has_tail) {
            array_buf_push(&statements, tail);
        }

        expression_t expression;
        if (parse_expression(parser, &expression) != SUCCESS) {
            skip_parser_invalid_statement(parser);
            has_tail = false;
            last_is_block = false;
            continue;
        }
        
        last_is_block = expression.kind == EXPRESSION_BLOCK
            || expression.kind == EXPRESSION_CONDITIONAL
            || expression.kind == EXPRESSION_INFINITE_LOOP
            || expression.kind == EXPRESSION_WHILE_LOOP;

        has_tail = true;
        tail = expression;
    }
    ast_push_alloc(&parser->storage, statements.data);

    *dst = (block_t){
        .statements = statements,
        .has_tail = has_tail,
        .tail = tail,
        .scope = NULL,
    };
    return SUCCESS;
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
    PRECEDENCE_MEMBER,
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

static void error_integer_too_big(
    parser_t* parser,
    parser_alloc_state_t state,
    text_view_t text
) {
    error_t error = {
        .kind = ERROR_INTEGER_TOO_BIG,
        .source = text,
        .message = format(
            "integer literal was too big (integer literals must be in the range -2^63 to 2^63 - 1)"
        )
    };
    parser_error_restore(parser, state, error);
}

static result_t parse_integer(parser_t* parser, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);

    // first token is integer when called
    text_view_t text = peek_parser(*parser).text;
    step_parser(parser);
    size_t i = 0;

    int64_t signum;
    switch (text.data[i]) {
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
            error_integer_too_big(parser, state, text);
            return ERROR;
        }
        int64_t signed_digit = (text.data[i] - '0') * signum;
        if (__builtin_add_overflow(value, signed_digit, &value)) {
            error_integer_too_big(parser, state, text);
            return ERROR;
        }
    }

    *dst = (expression_t){ .kind = EXPRESSION_INTEGER, .as.integer = value };
    return SUCCESS;
}

static result_t parse_symbol_expression(parser_t* parser, expression_t* dst) {
    // when called, first token is an identifier
    text_view_t name = peek_parser(*parser).text;
    step_parser(parser);
    *dst = (expression_t){ .kind = EXPRESSION_SYMBOL, .as.symbol.name = name };
    return SUCCESS;
}

static result_t parse_call(parser_t* parser, expression_t callee, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);
    token_t opening_paren = peek_parser(*parser);
    step_parser(parser); // skip "("

    expression_array_buf_t arguments = new_array_buf(expression_t);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        if (parser_is_eof(*parser)) {
            free_array_buf(arguments);
            error_t error = {
                .kind = ERROR_UNCLOSED_PARENS,
                .source = opening_paren.text,
                .message = format("unclosed parentheses `(`, `)`")
            };
            parser_error_restore(parser, state, error);
            return ERROR;
        }

        expression_t argument;
        if (parse_expression(parser, &argument) != SUCCESS) {
            token_type_t skip_until[2] = { TOKEN_COMMA, TOKEN_RIGHT_PAREN };
            skip_parser_until_any_of(parser, skip_until, 2);
            continue;
        }
        array_buf_push(&arguments, argument);

        if (match_parser(parser, TOKEN_COMMA, NULL)) {
            continue;
        }
    }

    ast_push_alloc(&parser->storage, arguments.data);
    call_t call = {
        .callee = ast_box(&parser->storage, callee),
        .arguments = arguments,
    };
    *dst = (expression_t){ .kind = EXPRESSION_CALL, .as.call = call };
    return SUCCESS;
}

static result_t parse_index(parser_t* parser, expression_t indexee, expression_t* dst) {
    expression_t index;
    token_t left_bracket;
    error_t error;
    token_t invalid_closing_token;
    switch (parse_delimited_expression(
        parser,
        TOKEN_LEFT_BRACKET,
        TOKEN_RIGHT_BRACKET,
        &index,
        &left_bracket,
        &invalid_closing_token
    )) {
    case DELIMITED_EMPTY:
        error = (error_t){
            .kind = ERROR_MISSING_INDEX,
            .source = text_view_disjoint_union(left_bracket.text, peek_parser(*parser).text),
            .message = format("missing index in index expression (empty brackets `[` `]`)")
        };
        return ERROR;

    case DELIMITED_INVALID_INSIDE:
        return ERROR;
    
    case DELIMITED_EXCESS_INSIDE:
        error = (error_t){
            .kind = ERROR_EXCESS_INDEX_TOKENS,
            .source = invalid_closing_token.text,
            .message = format("invalid token in index expression (expected closing bracket `]`)")
        };
        report(parser->reporter, error);
        return ERROR;

    case DELIMITED_UNCLOSED:
        error = (error_t){
            .kind = ERROR_UNCLOSED_BRACKETS,
            .source = left_bracket.text,
            .message = format("unclosed brackets `[` `]`"),
        };
        report(parser->reporter, error);
        return ERROR;

    // actually unreachable: this function only gets entered in `parse_expression_precendence`
    // when a left bracket `[` is present.
    case DELIMITED_UNOPENED: break;
    
    case DELIMITED_SUCCESS: break;
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
    return SUCCESS;
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

static result_t parse_expression_precedence(
    parser_t* super_parser,
    precedence_t precedence,
    expression_t* dst
);

static result_t parse_prefix(parser_t* parser, unary_operator_t operator, expression_t* dst) {
    precedence_t precedence = prefix_operator_precedence(operator);
    expression_t operand;
    if (parse_expression_precedence(parser, precedence, &operand) != SUCCESS) {
        return ERROR;
    }

    unary_operation_t operation = {
        .operator = operator,
        .operand = ast_box(&parser->storage, operand),
    };
    *dst = (expression_t) {
        .kind = EXPRESSION_UNARY_OPERATION,
        .as.unary_operation = operation
    };
    return SUCCESS;
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

static result_t parse_infix(
    parser_t* parser,
    expression_t lhs,
    binary_operator_t operator,
    expression_t* dst
) {
    precedence_t precedence = infix_operator_precedence(operator);
    expression_t rhs;
    if (parse_expression_precedence(parser, next_precedence(precedence), &rhs) != SUCCESS) {
        return ERROR;
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
    return SUCCESS;
}

static result_t parse_binding(parser_t* parser, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);

    step_parser(parser); // skip "let"

    variable_t variable;
    if (parse_variable(parser, &variable) != SUCCESS) {
        return ERROR;
    }

    if (!match_parser(parser, TOKEN_EQUAL, NULL)) {
        error_t error = {
            .kind = ERROR_MISSING_EQUALS_IN_BINDING,
            .source = peek_parser(*parser).text,
            .message = format("expected equals sign `=` in `let` binding")
        };
        parser_error_restore(parser, state, error);
        return ERROR;
    }

    expression_t value;
    if (parse_expression(parser, &value) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    binding_t binding = {
        .variable = variable,
        .value = ast_box(&parser->storage, value),
    };
    *dst = (expression_t){ .kind = EXPRESSION_BINDING, .as.binding = binding };
    return SUCCESS;
}

static result_t parse_conditional(parser_t* parser, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip `if` or `elif`

    expression_t condition;
    if (parse_expression(parser, &condition) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    block_t body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
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
            if (parse_block(parser, &else_block) != SUCCESS) {
                parser_restore(parser, state);
                return ERROR;
            }
            conditional.else_kind = CONDITIONAL_ELSE_BLOCK;
            conditional.else_as.block = ast_box(&parser->storage, else_block);
            break;

        case TOKEN_ELIF:
            ;
            expression_t else_conditional;
            if (parse_conditional(parser, &else_conditional) != SUCCESS) {
                parser_restore(parser, state);
                return ERROR;
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
    return SUCCESS;
}

static result_t parse_infinite_loop(parser_t* parser, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip `loop`

    block_t body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    infinite_loop_t infinite_loop = { .body = ast_box(&parser->storage, body) };
    *dst = (expression_t){ .kind = EXPRESSION_INFINITE_LOOP, .as.infinite_loop = infinite_loop };
    return SUCCESS;
}

static result_t parse_while_loop(parser_t* parser, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip "while"

    expression_t condition;
    if (parse_expression(parser, &condition) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    block_t body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    while_loop_t while_loop = {
        .condition = ast_box(&parser->storage, condition),
        .body = ast_box(&parser->storage, body)
    };
    *dst = (expression_t){
        .kind = EXPRESSION_WHILE_LOOP,
        .as.while_loop = while_loop
    };
    return SUCCESS;
}

static result_t parse_member_access(parser_t* parser, expression_t container, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);
    step_parser(parser); // skip `.`

    token_t field;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &field)) {
        error_t error = {
            .kind = ERROR_MEMBER_NOT_IDENTIFIER,
            .source = peek_parser(*parser).text,
            .message = format("member wasn't an identifier")
        };
        parser_error_restore(parser, state, error);
        return ERROR;
    }

    member_access_t member_access = {
        .container = ast_box(&parser->storage, container),
        .member_name = field.text,
    };
    *dst = (expression_t){
        .kind = EXPRESSION_MEMBER_ACCESS,
        .as.member_access = member_access
    };
    return SUCCESS;
}

static result_t parse_paren(parser_t* parser, expression_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);

    token_t opening_paren;
    expression_t inside;
    error_t error;
    token_t invalid_closing_token;
    switch (parse_delimited_expression(
        parser,
        TOKEN_LEFT_PAREN,
        TOKEN_RIGHT_PAREN,
        &inside,
        &opening_paren,
        &invalid_closing_token
    )) {
    case DELIMITED_EMPTY:
        // FIXME: unit expression
        *dst = (expression_t){
            .kind = EXPRESSION_UNIT,
            .type = (type_t){ .element_type = TYPE_UNIT, .array_depth = 0 }
        };
        return SUCCESS;
    
    case DELIMITED_EXCESS_INSIDE:
        error = (error_t){
            .kind = ERROR_EXCESS_TOKENS_IN_PARENS,
            .source = invalid_closing_token.text,
            .message = format("invalid token in parenthesized expression (expected closing parenthesis `)`)")
        };
        report(parser->reporter, error);
        // fallthrough
    case DELIMITED_INVALID_INSIDE:
        *dst = (expression_t){
            .kind = EXPRESSION_INVALID,
            .type = (type_t){ .element_type = TYPE_NEVER, .array_depth = 0 }
        };
        return SUCCESS;

    case DELIMITED_UNCLOSED:
        error = (error_t){
            .kind = ERROR_UNCLOSED_PARENS,
            .source = opening_paren.text,
            .message = format("unclosed parentheses `(` `)`"),
        };
        report(parser->reporter, error);
        return ERROR;

    // actually unreachable: this function only gets entered in `parse_expression_precendence`
    // when a left parenthesis `(` is present.
    case DELIMITED_UNOPENED: break;
    
    case SUCCESS: break;
    }

    *dst = inside;
    return SUCCESS;
}

static result_t parse_expression_precedence(
    parser_t* parser,
    precedence_t precedence,
    expression_t* dst
) {
    parser_alloc_state_t state = parser_snapshot(*parser);
    token_t start = peek_parser(*parser);
    result_t result = ERROR;
    expression_t expression;

    unary_operator_t unary_operator = as_prefix_operator(peek_parser(*parser));
    if (unary_operator != -1) {
        step_parser(parser);
        result = parse_prefix(parser, unary_operator, &expression);
    } else switch (peek_parser(*parser).type) {
    case TOKEN_INTEGER:
        result = parse_integer(parser, &expression);
        break;

    case TOKEN_IDENTIFIER:
        result = parse_symbol_expression(parser, &expression);
        break;

    case TOKEN_LEFT_PAREN:
        result = parse_paren(parser, &expression);
        break;

    case TOKEN_LEFT_BRACE:
        ;
        block_t block;
        if (parse_block(parser, &block) != SUCCESS) {
            parser_restore(parser, state);
            return ERROR;
        }
        result = SUCCESS;
        expression = (expression_t){
            .kind = EXPRESSION_BLOCK,
            .as.block = ast_box(&parser->storage, block)
        };
        break;

    case TOKEN_LET:
        result = parse_binding(parser, &expression);
        break;

    case TOKEN_IF:
        result = parse_conditional(parser, &expression);
        break;

    case TOKEN_LOOP:
        result = parse_infinite_loop(parser, &expression);
        break;

    case TOKEN_WHILE:
        result = parse_while_loop(parser, &expression);
        break;

    default: break;
    }

    if (result != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    if (precedence >= PRECEDENCE_MEMBER) {
        *dst = expression;
        return SUCCESS;
    }

    while (true) {
        switch (peek_parser(*parser).type) {
        case TOKEN_DOT:
            result = parse_member_access(parser, expression, &expression);
            break;
        case TOKEN_LEFT_PAREN:
            result = parse_call(parser, expression, &expression);
            break;
        case TOKEN_LEFT_BRACKET:
            result = parse_index(parser, expression, &expression);
            break;
        default:
            goto parse_binary_operation;
        }

        if (result != SUCCESS) {
            parser_restore(parser, state);
            return ERROR;
        }
    }
    
parse_binary_operation:
    while (true) {
        binary_operator_t binary_operator = as_infix_operator(peek_parser(*parser));
        if (binary_operator == -1) {
            break;
        }
        step_parser(parser);
        precedence_t operator_precedence = infix_operator_precedence(binary_operator);

        if (!precedence_compatible(precedence, operator_precedence)) {
            error_t error = {
                .kind = ERROR_INCOMPATIBLE_BINARY_OPERATIONS,
                .source = text_view_disjoint_union(start.text, peek_parser(*parser).text),
                .message = format(
                    "mixed binary operators with ambiguous precedence"
                    "(introduce parentheses `(` `)` to resolve ambiguity)"
                )
            };
            parser_error_restore(parser, state, error);
            return ERROR;
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

        result = parse_infix(parser, expression, binary_operator, &expression);
        if (result != SUCCESS) {
            parser_restore(parser, state);
            return ERROR;
        }
    }

    dst->type = (type_t){ .array_depth = 0, .element_type.kind = TYPE_NEVER };
    *dst = expression;
    return SUCCESS;
}

result_t parse_expression(parser_t* parser, expression_t* dst) {
    return parse_expression_precedence(parser, PRECEDENCE_NONE, dst);
}
