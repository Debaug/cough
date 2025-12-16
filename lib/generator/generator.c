#include "generator/generator.h"

typedef struct Generator {
    Emitter* emitter;
    Expression const* expressions;
    BindingRegistry bindings;
} Generator;

static void generate_function(Generator gen, Function* function);
static void generate_pattern_match(Generator gen, Pattern pattern);
static void generate_expression(Generator gen, Expression expression);

void generate(Ast* ast, Emitter* emitter) {
    Generator generator = {
        .emitter = emitter,
        .expressions = ast->expressions.data,
        .bindings = ast->bindings,
    };
    for (size_t i = 0; i < ast->root.global_constants.len; i++) {
        ConstantDef* constant_def = &ast->root.global_constants.data[i];
        Expression* value = &ast->expressions.data[constant_def->value];
        if (value->kind != EXPRESSION_FUNCTION) {
            continue;
        }
        generate_function(generator, &value->as.function);
    }
}

static void generate_function(Generator gen, Function* function) {
    function->symbol = emit_new_symbol(gen.emitter);
    if (function->variable_space > 0) {
        emit_res(gen.emitter, function->variable_space);
    }
    generate_pattern_match(gen, function->input);
    generate_expression(gen, gen.expressions[function->output]);
    emit_ret(gen.emitter);
}

static void generate_pattern_match(Generator gen, Pattern pattern) {
    switch (pattern.kind) {
    case PATTERN_VARIABLE:;
        BindingId binding = pattern.as.variable.binding;
        // TODO: sanity check if it's a value binding & variable store
        usize variable_index = get_binding(gen.bindings, binding)
            .as.value.store.as.variable_index;
        emit_set(gen.emitter, variable_index);
    }
}

static void generate_expression(Generator gen, Expression expression) {
    switch (expression.kind) {
    case EXPRESSION_VARIABLE:;
        BindingId binding = expression.as.variable.binding;
        // TODO: sanity check if it's a value binding & variable store
        usize variable_index = get_binding(gen.bindings, binding)
            .as.value.store.as.variable_index;
        emit_var(gen.emitter, variable_index);
        break;
    
    case EXPRESSION_FUNCTION:
        // TODO: generate function expression
        exit(-1);
        break;

    case EXPRESSION_LITERAL_BOOL:
        emit_sca(
            gen.emitter,
            (Word){ .as_uint = expression.as.literal_bool }
        );
        break;
    }
}
