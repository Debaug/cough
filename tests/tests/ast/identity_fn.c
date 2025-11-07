#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include "analyzer/analyzer.h"

#include "tests/common.h"

int main(int argc, char const *argv[]) {
    char const source_raw[] = 
        "identity :: fn x: Bool -> Bool => x;\n"
    ;
    String source = STRING_LITERAL(source_raw);

    TestReporter reporter = test_reporter_new();

    TokenStream tokens;
    assert(tokenize(source, &reporter.base, &tokens));
    Ast ast;
    assert(parse(tokens, &reporter.base, &ast));
    assert(analyze(&ast, &reporter.base));

    // we don't check everything -- a more thorough test is conducted
    // by `ast/constant_fn`.
    
    assert(ast.root.global_constants.len == 1);
    ConstantDef identity_def = ast.root.global_constants.data[0];

    TypeId fn_bool_bool_type = get_or_register_function_type(
        &ast.types,
        (FunctionType){ .input = TYPE_BOOL, .output = TYPE_BOOL }
    );
    assert(identity_def.type == fn_bool_bool_type);

    Expression value = ast.expressions.data[identity_def.value];
    assert(value.kind == EXPRESSION_FUNCTION);
    Function identity = value.as.function;

    assert(identity.input.kind == PATTERN_VARIABLE);
    BindingId x_binding_id = identity.input.as.variable.binding;
    Binding x_binding = get_binding(ast.bindings, x_binding_id);
    assert(x_binding.kind == BINDING_VALUE);
    assert(eq(String)(x_binding.as.value.name, STRING_LITERAL("x")));
    assert(x_binding.as.value.type == TYPE_BOOL);
    assert(!x_binding.as.value.constant);

    return 0;
}
