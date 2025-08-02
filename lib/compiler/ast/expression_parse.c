#include "compiler/diagnostics.h"
#include "compiler/ast/expression.h"

typedef enum ParseDelimitedResult {
    DELIMITED_SUCCESS,
    DELIMITED_UNOPENED,
    DELIMITED_EMPTY,
    DELIMITED_INVALID_INSIDE,
    DELIMITED_EXCESS_INSIDE,
    DELIMITED_UNCLOSED,
} ParseDelimitedResult;

static ParseDelimitedResult parse_delimited_expression(
    Parser* parser,
    TokenKind opening,
    TokenKind closing,
    Expression* dst,
    Token* opening_token,
    Token* invalid_closing_token
) {
    if (!match_parser(parser, opening, opening_token)) {
        return DELIMITED_UNOPENED;
    }
    if (match_parser(parser, closing, NULL)) {
        return DELIMITED_EMPTY;
    }

    Expression inside;
    ParserAllocState state = parser_snapshot_alloc(*parser);
    ParseDelimitedResult result;
    if (parse_expression(parser, &inside) != SUCCESS) {
        result = DELIMITED_INVALID_INSIDE;
    } else if (!match_parser(parser, closing, NULL)) {
        *invalid_closing_token = peek_parser(*parser);
        result = DELIMITED_EXCESS_INSIDE;
    } else {
        result = DELIMITED_SUCCESS;
    }

    if (result == DELIMITED_INVALID_INSIDE || result == DELIMITED_EXCESS_INSIDE) {
        parser_restore_alloc(parser, state);
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

static void skip_parser_invalid_statement(Parser* parser) {
    TokenKind skip_until[2] = { TOKEN_RIGHT_BRACE, TOKEN_SEMICOLON };
    skip_parser_until_any_of(parser, skip_until, 2);
}

Result parse_block(Parser* parser, Block* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);

    Token opening_brace;
    if (!match_parser(parser, TOKEN_LEFT_BRACE, &opening_brace)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_EXPECTED_BLOCK,
            format("expected block (delimited by curly braces `{`, `}`)"),
            peek_parser(*parser).text
        );
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    ExpressionArrayBuf statements = new_array_buf(Expression);
    bool has_tail = false;
    Expression tail = {0};
    bool last_is_block = false;
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        if (parser_is_eof(*parser)) {
            report_simple_compiler_error(
                parser->reporter,
                CE_UNCLOSED_BRACES,
                format("unclosed block (delimited by curly braces `{` `}`)"),
                opening_brace.text
            );
            parser_restore_alloc(parser, state);
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
            report_simple_compiler_error(
                parser->reporter,
                CE_EXCESS_EXPRESSION_TOKENS,
                format("statements must be separated by a semicolon `;`"),
                peek_parser(*parser).text
            );
            continue;
        }
        if (has_tail) {
            array_buf_push(&statements, tail);
        }

        Expression expression;
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

    *dst = (Block){
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
typedef enum Precedence {
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
} Precedence;

static bool precedence_compatible(Precedence lhs, Precedence rhs) {
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

static Precedence next_precedence(Precedence precedence) {
    if (precedence == PRECEDENCE_BITWISE) {
        return PRECEDENCE_UNARY;
    }
    return precedence + 1;
}

static void error_integer_too_big(
    Parser* parser,
    ParserAllocState state,
    TextView text
) {
    report_simple_compiler_error(
        parser->reporter,
        CE_INTEGER_TOO_BIG,
        format(
            "integer literal was too big (integer literals must be in the range -2^63 to 2^63 - 1)"
        ),
        text
    );
    parser_restore_alloc(parser, state);
}

static Result parse_integer(Parser* parser, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);

    // first token is integer when called
    TextView text = peek_parser(*parser).text;
    step_parser(parser);
    usize i = 0;

    i64 signum;
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

    i64 value = 0;
    for (; i < text.len; i++) {
        if (__builtin_mul_overflow(value, 10, &value)) {
            error_integer_too_big(parser, state, text);
            return ERROR;
        }
        i64 signed_digit = (text.data[i] - '0') * signum;
        if (__builtin_add_overflow(value, signed_digit, &value)) {
            error_integer_too_big(parser, state, text);
            return ERROR;
        }
    }

    *dst = (Expression){ .kind = EXPRESSION_INTEGER, .as.integer = value };
    return SUCCESS;
}

static Result parse_symbol_expression(Parser* parser, Expression* dst) {
    // when called, first token is an identifier
    TextView name = peek_parser(*parser).text;
    step_parser(parser);
    *dst = (Expression){ .kind = EXPRESSION_SYMBOL, .as.symbol.name = name };
    return SUCCESS;
}

static Result parse_call(Parser* parser, Expression callee, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);
    Token opening_paren = peek_parser(*parser);
    step_parser(parser); // skip "("

    ExpressionArrayBuf arguments = new_array_buf(Expression);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        if (parser_is_eof(*parser)) {
            free_array_buf(arguments);
            report_simple_compiler_error(
                parser->reporter,
                CE_UNCLOSED_PARENS,
                format("unclosed parentheses `(`, `)`"),
                opening_paren.text
            );
            parser_restore_alloc(parser, state);
            return ERROR;
        }

        Expression argument;
        if (parse_expression(parser, &argument) != SUCCESS) {
            TokenKind skip_until[2] = { TOKEN_COMMA, TOKEN_RIGHT_PAREN };
            skip_parser_until_any_of(parser, skip_until, 2);
            continue;
        }
        array_buf_push(&arguments, argument);

        if (match_parser(parser, TOKEN_COMMA, NULL)) {
            continue;
        }
    }

    ast_push_alloc(&parser->storage, arguments.data);
    Call call = {
        .callee = ast_box(&parser->storage, callee),
        .arguments = arguments,
    };
    *dst = (Expression){ .kind = EXPRESSION_CALL, .as.call = call };
    return SUCCESS;
}

static Result parse_index(Parser* parser, Expression indexee, Expression* dst) {
    Expression index;
    Token left_bracket;
    Token invalid_closing_token;
    switch (parse_delimited_expression(
        parser,
        TOKEN_LEFT_BRACKET,
        TOKEN_RIGHT_BRACKET,
        &index,
        &left_bracket,
        &invalid_closing_token
    )) {
    case DELIMITED_EMPTY:
        report_simple_compiler_error(
            parser->reporter,
            CE_MISSING_EXPRESSION,
            format("missing index in index expression (empty brackets `[` `]`)"),
            text_view_disjoint_union(left_bracket.text, peek_parser(*parser).text)
        );
        return ERROR;

    case DELIMITED_INVALID_INSIDE:
        return ERROR;
    
    case DELIMITED_EXCESS_INSIDE:
        report_simple_compiler_error(
            parser->reporter,
            CE_EXCESS_EXPRESSION_TOKENS,
            format("invalid token in index expression (expected closing bracket `]`)"),
            invalid_closing_token.text
        );
        return ERROR;

    case DELIMITED_UNCLOSED:
        report_simple_compiler_error(
            parser->reporter,
            CE_UNCLOSED_BRACKETS,
            format("unclosed brackets `[` `]`"),
            left_bracket.text
        );
        return ERROR;

    // actually unreachable: this function only gets entered in `parse_expression_precendence`
    // when a left bracket `[` is present.
    case DELIMITED_UNOPENED: break;
    
    case DELIMITED_SUCCESS: break;
    }

    BinaryOperation operation = {
        .operator = OPERATOR_INDEX,
        .left = ast_box(&parser->storage, indexee),
        .right = ast_box(&parser->storage, index),
    };
    *dst = (Expression){
        .kind = EXPRESSION_BINARY_OPERATION,
        .as.binary_operation = operation
    };
    return SUCCESS;
}

static UnaryOperator as_prefix_operator(Token token) {
    switch (token.kind) {
    case TOKEN_MINUS: return OPERATOR_NEGATE;
    case TOKEN_BANG: return OPERATOR_NOT;
    case TOKEN_RETURN: return OPERATOR_RETURN;
    case TOKEN_BREAK: return OPERATOR_BREAK;
    default: return -1;
    }
}

static Precedence prefix_operator_precedence(UnaryOperator operator) {
    switch (operator) {
    case OPERATOR_NOT:
    case OPERATOR_NEGATE:
        return PRECEDENCE_UNARY;
    case OPERATOR_RETURN:
    case OPERATOR_BREAK:
        return PRECEDENCE_DIVERGE;
    }
}

static Result parse_expression_precedence(
    Parser* super_parser,
    Precedence precedence,
    Expression* dst
);

static Result parse_prefix(Parser* parser, UnaryOperator operator, Expression* dst) {
    Precedence precedence = prefix_operator_precedence(operator);
    Expression operand;
    if (parse_expression_precedence(parser, precedence, &operand) != SUCCESS) {
        return ERROR;
    }

    UnaryOperation operation = {
        .operator = operator,
        .operand = ast_box(&parser->storage, operand),
    };
    *dst = (Expression) {
        .kind = EXPRESSION_UNARY_OPERATION,
        .as.unary_operation = operation
    };
    return SUCCESS;
}

static BinaryOperator as_infix_operator(Token token) {
    switch (token.kind) {
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

static Precedence infix_operator_precedence(BinaryOperator operator) {
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

static Result parse_infix(
    Parser* parser,
    Expression lhs,
    BinaryOperator operator,
    Expression* dst
) {
    Precedence precedence = infix_operator_precedence(operator);
    Expression rhs;
    if (parse_expression_precedence(parser, next_precedence(precedence), &rhs) != SUCCESS) {
        return ERROR;
    }

    BinaryOperation operation = {
        .operator = operator,
        .left = ast_box(&parser->storage, lhs),
        .right = ast_box(&parser->storage, rhs),
    };
    *dst = (Expression){
        .kind = EXPRESSION_BINARY_OPERATION,
        .as.binary_operation = operation
    };
    return SUCCESS;
}

static Result parse_binding(Parser* parser, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);

    step_parser(parser); // skip "let"

    Variable variable;
    if (parse_variable(parser, &variable) != SUCCESS) {
        return ERROR;
    }

    if (!match_parser(parser, TOKEN_EQUAL, NULL)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_MISSING_EXPRESSION,
            format("expected equals sign `=` in `let` binding"),
            peek_parser(*parser).text
        );
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Expression value;
    if (parse_expression(parser, &value) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Binding binding = {
        .variable = variable,
        .value = ast_box(&parser->storage, value),
    };
    *dst = (Expression){ .kind = EXPRESSION_BINDING, .as.binding = binding };
    return SUCCESS;
}

static Result parse_conditional(Parser* parser, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);
    step_parser(parser); // skip `if` or `elif`

    Expression condition;
    if (parse_expression(parser, &condition) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Block body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Conditional conditional = {
        .else_kind = CONDITIONAL_ELSE_NONE,
        .else_as = {0}
    };
    while (true) {
        switch (peek_parser(*parser).kind) {
        case TOKEN_ELSE:
            step_parser(parser);
            Block else_block;
            if (parse_block(parser, &else_block) != SUCCESS) {
                parser_restore_alloc(parser, state);
                return ERROR;
            }
            conditional.else_kind = CONDITIONAL_ELSE_BLOCK;
            conditional.else_as.block = ast_box(&parser->storage, else_block);
            break;

        case TOKEN_ELIF:
            ;
            Expression else_conditional;
            if (parse_conditional(parser, &else_conditional) != SUCCESS) {
                parser_restore_alloc(parser, state);
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

    *dst = (Expression){
        .kind = EXPRESSION_CONDITIONAL,
        .as.conditional = conditional
    };
    return SUCCESS;
}

static Result parse_infinite_loop(Parser* parser, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);
    step_parser(parser); // skip `loop`

    Block body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    InfiniteLoop infinite_loop = { .body = ast_box(&parser->storage, body) };
    *dst = (Expression){ .kind = EXPRESSION_INFINITE_LOOP, .as.infinite_loop = infinite_loop };
    return SUCCESS;
}

static Result parse_while_loop(Parser* parser, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);
    step_parser(parser); // skip "while"

    Expression condition;
    if (parse_expression(parser, &condition) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Block body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    WhileLoop while_loop = {
        .condition = ast_box(&parser->storage, condition),
        .body = ast_box(&parser->storage, body)
    };
    *dst = (Expression){
        .kind = EXPRESSION_WHILE_LOOP,
        .as.while_loop = while_loop
    };
    return SUCCESS;
}

static Result parse_member_access(Parser* parser, Expression container, Expression* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);
    step_parser(parser); // skip `.`

    Token field;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &field)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_MEMBER_NOT_IDENTIFIER,
            format("member wasn't an identifier"),
            peek_parser(*parser).text
        );
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    MemberAccess member_access = {
        .container = ast_box(&parser->storage, container),
        .member_name = field.text,
    };
    *dst = (Expression){
        .kind = EXPRESSION_MEMBER_ACCESS,
        .as.member_access = member_access
    };
    return SUCCESS;
}

static Result parse_paren(Parser* parser, Expression* dst) {
    // FIXME: check if all allocations are properly freed

    Token opening_paren;
    Expression inside;
    Token invalid_closing_token;
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
        *dst = (Expression){
            .kind = EXPRESSION_UNIT,
            .type = (Type){ .element_type = TYPE_UNIT, .array_depth = 0 }
        };
        return SUCCESS;
    
    case DELIMITED_EXCESS_INSIDE:
        report_simple_compiler_error(
            parser->reporter,
            CE_EXCESS_EXPRESSION_TOKENS,
            format("invalid token in parenthesized expression (expected closing parenthesis `)`)"),
            invalid_closing_token.text
        );
        // fallthrough
    case DELIMITED_INVALID_INSIDE:
        *dst = (Expression){
            .kind = EXPRESSION_INVALID,
            .type = (Type){ .element_type = TYPE_NEVER, .array_depth = 0 }
        };
        return SUCCESS;

    case DELIMITED_UNCLOSED:
        report_simple_compiler_error(
            parser->reporter,
            CE_UNCLOSED_PARENS,
            format("unclosed parentheses `(` `)`"),
            opening_paren.text
        );
        return ERROR;

    // actually unreachable: this function only gets entered in `parse_expression_precendence`
    // when a left parenthesis `(` is present.
    case DELIMITED_UNOPENED: break;
    
    case SUCCESS: break;
    }

    *dst = inside;
    return SUCCESS;
}

static Result parse_expression_precedence(
    Parser* parser,
    Precedence precedence,
    Expression* dst
) {
    ParserAllocState state = parser_snapshot_alloc(*parser);
    Token start = peek_parser(*parser);
    Result result = ERROR;
    Expression expression;

    UnaryOperator unary_operator = as_prefix_operator(peek_parser(*parser));
    if (unary_operator != -1) {
        step_parser(parser);
        result = parse_prefix(parser, unary_operator, &expression);
    } else switch (peek_parser(*parser).kind) {
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
        Block block;
        if (parse_block(parser, &block) != SUCCESS) {
            parser_restore_alloc(parser, state);
            return ERROR;
        }
        result = SUCCESS;
        expression = (Expression){
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
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    if (precedence >= PRECEDENCE_MEMBER) {
        *dst = expression;
        return SUCCESS;
    }

    while (true) {
        switch (peek_parser(*parser).kind) {
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
            parser_restore_alloc(parser, state);
            return ERROR;
        }
    }
    
parse_binary_operation:
    while (true) {
        BinaryOperator binary_operator = as_infix_operator(peek_parser(*parser));
        if (binary_operator == -1) {
            break;
        }
        step_parser(parser);
        Precedence operator_precedence = infix_operator_precedence(binary_operator);

        if (!precedence_compatible(precedence, operator_precedence)) {
            report_simple_compiler_error(
                parser->reporter,
                CE_INCOMPATIBLE_BINARY_OPERATIONS,
                format(
                    "mixed binary operators with ambiguous precedence "
                    "(introduce parentheses `(` `)` to resolve ambiguity)"
                ),
                text_view_disjoint_union(start.text, peek_parser(*parser).text)
            );
            parser_restore_alloc(parser, state);
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
            parser_restore_alloc(parser, state);
            return ERROR;
        }
    }

    dst->type = (Type){ .array_depth = 0, .element_type.kind = TYPE_NEVER };
    *dst = expression;
    return SUCCESS;
}

Result parse_expression(Parser* parser, Expression* dst) {
    return parse_expression_precedence(parser, PRECEDENCE_NONE, dst);
}
