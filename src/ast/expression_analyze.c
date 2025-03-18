#include "ast/expression.h"
#include "ast/function.h"

static Result analyze_symbol_expression(
    Analyzer* analyzer,
    SymbolExpression* symbol_expression,
    Type* dst
) {
    eprintf("TODO: analyze_symbol_expression\n");
    exit(EXIT_FAILURE);
    return SUCCESS;

    // const Symbol* symbol = find_symbol_of(
    //     *analyzer->current_scope,
    //     STRING_VIEW(symbol_expression->name),
    //     SYMBOL_VARIABLE | SYMBOL_FUNCTION
    // );

    // if (symbol == NULL) {
    //     // FIXME: error reporting
    //     // report_text_error(
    //     //     symbol_expression->name,
    //     //     "no variable or function named `%.*s`",
    //     //     TEXT_FMT(symbol_expression->name)
    //     // );
    //     return ANALYZE_ERROR;
    // }

    // if (symbol->kind == SYMBOL_VARIABLE) {
    //     symbol_expression->kind = SYMBOL_EXPRESSION_VARIABLE;
    //     symbol_expression->as.variable = symbol->as.variable;
    //     *dst = symbol->as.variable->type.type;
    // } else {
    //     symbol_expression->kind = SYMBOL_EXPRESSION_FUNCTION;
    //     symbol_expression->as.function = symbol->as.function;
    //     ElementType element_type = {
    //         .kind = TYPE_FUNCTION,
    //         .as.function_signature = &symbol->as.function->signature
    //     };
    //     *dst = (Type){ .array_depth = 0, .element_type = element_type };
    // }

    // return ANALYZE_SUCCESS;
}

#if 0
Result analyze_block(
    Analyzer* analyzer,
    block_t* block,
    Type* return_type,
    Type* break_type
) {
    block->scope = analyzer_enter_new_scope(analyzer);
    for (usize i = 0; i < block->statements.len; i++) {
        analyze_expression(
            analyzer,
            &block->statements.data[i],
            return_type,
            break_type
        );
    }
    if (block->has_tail) {
        analyze_expression(
            analyzer,
            &block->tail,
            return_type,
            break_type
        );
    }
    analyzer_exit_scope(analyzer);
    return ANALYZE_SUCCESS;
}
#endif

#define TYPE_IS_SCALAR(type, k) (                                                   \
    (type).array_depth == 0                                                         \
    && ((type).element_type.kind == k || (type).element_type.kind == TYPE_NEVER)    \
)

#define TYPE_IS_NUMERIC(type) (                     \
    (type).array_depth == 0                         \
    && (                                            \
        (type).element_type.kind == TYPE_INT        \
        || (type).element_type.kind == TYPE_FLOAT   \
        || (type).element_type.kind == TYPE_NEVER   \
    )                                               \
)

#define TYPE_IS_COMPOSITE(type) (                   \
    (type).array_depth == 0                         \
    && (                                            \
        (type).element_type.kind == TYPE_STRUCT     \
        || (type).element_type.kind == TYPE_VARIANT \
    )                                               \
)

#if 0
static analyze_result_t analyze_not(
    UnaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_not\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (!TYPE_IS_SCALAR(operation.operand->type, TYPE_BOOL)) {
        // FIXME: error reporting
        // report_error("operand to not operator must be of type `Bool`");
        return ANALYZE_ERROR;
    }
    *dst = (Type){ .array_depth = 0, .element_type.kind = TYPE_BOOL };
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_negate(
    UnaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_negate\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (!TYPE_IS_NUMERIC(operation.operand->type)) {
        // FIXME: where error?
        // report_error("operand to negation operator must be of numeric type");
        return ANALYZE_SUCCESS;
    }
    *dst = operation.operand->type;
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_diverge(
    UnaryOperation operation,
    Type* diverge_type
) {
    eprintf("TODO: analyze_diverge\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (diverge_type == NULL) {
        // FIXME: where error?
        // eport_error("dangling `break` or `return`");
    }
    if (TYPE_IS_SCALAR(*diverge_type, TYPE_NEVER)) {
        *diverge_type = operation.operand->type;
        return ANALYZE_SUCCESS;
    }
    if (!type_eq(*diverge_type, operation.operand->type)) {
        // FIXME: where error?
        // report_error("invalid operand type for `break` or `return`");
        return ANALYZE_ERROR;
    }
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_unary_operation(
    Analyzer* analyzer,
    UnaryOperation* operation,
    Type* return_type,
    Type* break_type,
    Type* dst
) {
    if (analyze_expression(
        analyzer,
        operation->operand,
        return_type,
        break_type
    ) != ANALYZE_SUCCESS) {
        return ANALYZE_ERROR;
    }
    switch (operation->operator) {
    case OPERATOR_NOT: return analyze_not(*operation, dst);
    case OPERATOR_NEGATE: return analyze_negate(*operation, dst);
    case OPERATOR_RETURN: return analyze_diverge(*operation, return_type);
    case OPERATOR_BREAK: return analyze_diverge(*operation, break_type);
    }
}

static analyze_result_t analyze_binary_arithmetic(
    BinaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_binary_arithmetic\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (!TYPE_IS_NUMERIC(operation.left->type)
        || !TYPE_IS_NUMERIC(operation.right->type)) {
        // FIXME: where error?
        // report_error("operands to arithmetic operation must be of type `Int` or `Float`");
        return ANALYZE_ERROR;
    }
    if (!type_eq(operation.left->type, operation.right->type)) {
        // FIXME: where error?
        // report_error("operands to binary arithmetic operations must be of the same type");
        return ANALYZE_ERROR;
    }
    *dst = operation.left->type;
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_binary_bitwise(
    BinaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_binary_bitwise\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (!TYPE_IS_SCALAR(operation.left->type, TYPE_BOOL)
        || !TYPE_IS_SCALAR(operation.left->type, TYPE_INT)
        || !TYPE_IS_SCALAR(operation.right->type, TYPE_BOOL)
        || !TYPE_IS_SCALAR(operation.right->type, TYPE_INT)) {
        // FIXME: where error?
        // report_error("operands to bitwise operations must be of type `Int` or `Bool`");
        return ANALYZE_ERROR;
    }
    if (!type_eq(operation.left->type, operation.right->type)) {
        // FIXME: where error?
        // report_error("operands to binary bitwise operations must be of the same type");
        return ANALYZE_ERROR;
    }
    *dst = operation.left->type;
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_binary_logical(
    BinaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_binary_logical\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (!TYPE_IS_SCALAR(operation.left->type, TYPE_BOOL)
        || !TYPE_IS_SCALAR(operation.right->type, TYPE_BOOL)) {
        // FIXME: where error?
        // report_error("operands to `and` or `or` must be of type `Bool`");
        return ANALYZE_ERROR;
    }
    *dst = operation.left->type;
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_comparison(
    BinaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_comparison\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (!TYPE_IS_NUMERIC(operation.left->type)
        || !TYPE_IS_NUMERIC(operation.right->type)) {
        // FIXME: where error?
        // report_error("operands to comparison must be of type `Int` or `Float`");
        return ANALYZE_ERROR;
    }
    if (!type_eq(operation.left->type, operation.right->type)) {
        // FIXME: where error?
        // report_error("operands to comparison must be of the same type");
        return ANALYZE_ERROR;
    }
    *dst = (Type){ .array_depth = 0, .element_type.kind = TYPE_BOOL };
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_index(
    BinaryOperation operation,
    Type* dst
) {
    eprintf("TODO: analyze_index\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (operation.left->type.array_depth == 0) {
        // FIXME: where error?
        // report_error("indexee must be an array");
        return ANALYZE_ERROR;
    }
    if (!TYPE_IS_SCALAR(operation.right->type, TYPE_INT)) {
        // FIXME: where error?
        // report_error("index must be of type `Int`");
        return ANALYZE_ERROR;
    }
    *dst = operation.left->type;
    dst->array_depth--;
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_binary_operation(
    Analyzer* analyzer,
    BinaryOperation* operation,
    Type* return_type,
    Type* break_type,
    Type* dst
) {
    eprintf("TODO: analyze_binary_operation\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    analyze_expression(analyzer, operation->left, return_type, break_type);
    analyze_expression(analyzer, operation->right, return_type, break_type);

    switch (operation->operator) {
    case OPERATOR_ADD:
    case OPERATOR_SUBTRACT:
    case OPERATOR_MULTIPLY:
    case OPERATOR_DIVIDE:
    case OPERATOR_REMAINDER:
        return analyze_binary_arithmetic(*operation, dst);

    case OPERATOR_BITWISE_OR:
    case OPERATOR_BITWISE_AND:
    case OPERATOR_BITWISE_XOR:
        return analyze_binary_bitwise(*operation, dst);

    case OPERATOR_LOGICAL_OR:
    case OPERATOR_LOGICAL_AND:
        return analyze_binary_logical(*operation, dst);
        
    case OPERATOR_LESS:
    case OPERATOR_LESS_EQUAL:
    case OPERATOR_EQUAL:
    case OPERATOR_NOT_EQUAL:
    case OPERATOR_GREATER:
    case OPERATOR_GREATER_EQUAL:
        return analyze_comparison(*operation, dst);

    case OPERATOR_INDEX:
        return analyze_index(*operation, dst);

    case OPERATOR_ASSIGN:
        // TODO:
        // report_error("TODO: assignment");
        exit(-1);
    }
}

static analyze_result_t
analyze_member_access(
    Analyzer* analyzer,
    MemberAccess* member,
    Type* return_type,
    Type* break_type,
    Type* dst
) {
    eprintf("TODO: analyze_binary_operation\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    if (analyze_expression(analyzer, member->container, return_type, break_type) != ANALYZE_SUCCESS) {
        return ANALYZE_ERROR;
    }
    Type container_type = member->container->type;
    if (!TYPE_IS_COMPOSITE(container_type)) {
        // report_error("can't access a field of a non-composite type");
        return ANALYZE_ERROR;
    }
    member->field = find_field(
        *container_type.element_type.as.composite,
        STRING_VIEW(member->member_name)
    );
    if (member->field == NULL) {
        // report_error("no such field `%.*s`", TEXT_FMT(member->member_name));
        return ANALYZE_ERROR;
    }
    *dst = member->field->type.type;
    return ANALYZE_SUCCESS;
}

static analyze_result_t analyze_call(
    Analyzer* analyzer,
    Call* call,
    Type* return_type,
    Type* break_type,
    Type* dst
) {
    eprintf("TODO: analyze_call\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    analyze_result_t result = ANALYZE_SUCCESS;
    if (analyze_expression(analyzer, call->callee, return_type, break_type) != ANALYZE_SUCCESS) {
        result = ANALYZE_ERROR;
    }
    if (call->callee->type.array_depth != 0 || call->callee->type.element_type.kind != TYPE_FUNCTION) {
        // report_error("can't call non-function value");
        result = ANALYZE_ERROR;
    }
    for (usize i = 0; i < call->arguments.len; i++) {
        if (analyze_expression(analyzer, &call->arguments.data[i], return_type, break_type) != ANALYZE_SUCCESS) {
            result = ANALYZE_ERROR;
        }
        if (!type_eq(
            call->arguments.data[i].type,
            call->callee->type.element_type.as.function_signature->parameters.data[i].type.type
        )) {
            // report_error("argument and parameter type do not match");
            result = ANALYZE_ERROR;
        }
    }
    if (call->callee->type.element_type.as.function_signature->has_return_type) {
        *dst = call->callee->type.element_type.as.function_signature->return_type.type;
    } else {
        *dst = (Type){ .array_depth = 0, .element_type.kind = TYPE_UNIT }; 
    }
    return result;
}

// static analyze_result_t analyze_binding(
//     Analyzer* analyzer,
//     Binding* binding,
//     Type* return_type,
//     Type* break_type,
//     Type* dst
// ) {
//     Symbol symbol = {
//         .name = binding->variable.name,
//         .kind = SYMBOL_VARIABLE,
//         .as.variable = &binding->variable,
//     };
//     if (!add_symbol(analyzer->current_scope, symbol)) {
//         report_error(
//             "variable `%.*s` defined multiple times in the same scope",
//             TEXT_FMT(binding->variable.name)
//         );
//     }
// }

analyze_result_t analyze_expression(
    Analyzer* analyzer,
    Expression* expression,
    Type* return_type,
    Type* break_type
) {
    eprintf("TODO: analyze_expression\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    switch (expression->kind) {
    case EXPRESSION_INTEGER:
        expression->type = (Type){
            .array_depth = 0,
            .element_type.kind = TYPE_INT
        };
        return ANALYZE_SUCCESS;

    case EXPRESSION_SYMBOL:
        return analyze_symbol_expression(
            analyzer,
            &expression->as.symbol,
            &expression->type
        );

    case EXPRESSION_BLOCK: ;
        analyze_result_t result = analyze_block(
            analyzer,
            expression->as.block,
            return_type,
            break_type
        );
        if (result != ANALYZE_SUCCESS) {
            return result;
        }
        expression->type = (expression->as.block->has_tail)
            ? expression->as.block->tail.type
            : (Type){ .array_depth = 0, .element_type.kind = TYPE_UNIT };
        return ANALYZE_SUCCESS;

    case EXPRESSION_UNARY_OPERATION:
        return analyze_unary_operation(
            analyzer,
            &expression->as.unary_operation,
            return_type,
            break_type,
            &expression->type
        );

    case EXPRESSION_BINARY_OPERATION:
        return analyze_binary_operation(
            analyzer,
            &expression->as.binary_operation,
            return_type,
            break_type,
            &expression->type
        );

    case EXPRESSION_MEMBER_ACCESS:
        return analyze_member_access(
            analyzer,
            &expression->as.member_access,
            return_type,
            break_type,
            &expression->type
        );

    case EXPRESSION_CALL:
        return analyze_call(
            analyzer,
            &expression->as.call,
            return_type,
            break_type,
            &expression->type
        );

    case EXPRESSION_BINDING:
    case EXPRESSION_CONDITIONAL:
    case EXPRESSION_INFINITE_LOOP:
    case EXPRESSION_WHILE_LOOP:
        // TODO:
        // report_error("TODO: analyze expressions");
        exit(-1);
    }
}
#endif
