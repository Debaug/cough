#include "tests/common.h"

int main(int argc, char const** argv) {
    Ast ast = source_to_ast(STRING_LITERAL(
        "wrap :: fn x: Bool -> Bool => identity(x);\n"
        "identity :: fn y: Bool -> Bool => y;\n"
    ));
    
    assert(ast.root.global_constants.len == 2);

    // we don't repeat tests conducted in ast/identity_fn

    ConstantDef wrap_def = ast.root.global_constants.data[0];
    assert(eq(String)(wrap_def.name.string, STRING_LITERAL("wrap")));
    Expression wrap_value = ast.expressions.data[wrap_def.value];
    assert(wrap_value.kind == EXPRESSION_FUNCTION);
    Function wrap_fn = wrap_value.as.function;
    assert(wrap_fn.input.type == TYPE_BOOL);
    assert(wrap_fn.output_type == TYPE_BOOL);
    Expression output = ast.expressions.data[wrap_fn.output];
    assert(output.kind == EXPRESSION_BINARY_OPERATION);
    BinaryOperation call = output.as.binary_operation;
    assert(call.operator == OPERATION_FUNCTION_CALL);
    Expression lhs = ast.expressions.data[call.operand_left];
    assert(lhs.kind == EXPRESSION_VARIABLE);
    Binding lhs_binding = get_binding(ast.bindings, lhs.as.variable.binding);
    assert(lhs_binding.kind == BINDING_VALUE);
    assert(eq(String)(lhs_binding.as.value.name, STRING_LITERAL("identity")));
    Expression rhs = ast.expressions.data[call.operand_right];
    assert(rhs.kind == EXPRESSION_VARIABLE);
    Binding rhs_binding = get_binding(ast.bindings, rhs.as.variable.binding);
    assert(eq(String)(rhs_binding.as.value.name, STRING_LITERAL("x")));

    return 0;
}
