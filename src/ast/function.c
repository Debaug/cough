#include "ast/function.h"

result_t parse_function(parser_t* parser, function_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);

    function_signature_t signature;
    if (parse_function_signature(parser, &signature) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    block_t body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore(parser, state);
        return ERROR;
    }

    *dst = (function_t){
        .signature = signature,
        .body = body
    };
    return SUCCESS;
}

result_t analyze_function(analyzer_t* analyzer, function_t* function) {
    eprintf("TODO: analyze_function\n");
    exit(EXIT_FAILURE);
    return SUCCESS;

    // scope_t* scope = analyzer_enter_new_scope(analyzer);

    // bool error = false;
    // for (size_t i = 0; i < function->signature.parameters.len; i++) {
    //     variable_t* parameter = &function->signature.parameters.data[i];
    //     const symbol_t* type_symbol = find_symbol_of(
    //         *analyzer->current_scope,
    //         STRING_VIEW(parameter->type.element_type_name),
    //         SYMBOL_TYPE
    //     );
    //     if (type_symbol == NULL) {
    //         // FIXME: where error?
    //         report_error("unknown parameter type");
    //         error = true;
    //     } else {
    //         parameter->type.type.element_type = type_symbol->as.type;
    //     }
        
    //     symbol_t parameter_symbol = {
    //         .name = parameter->name,
    //         .kind = SYMBOL_VARIABLE,
    //         .as.variable = parameter
    //     };
    //     if (!add_symbol(scope, parameter_symbol)) {
    //         // FIXME: where error?
    //         report_error("parameter with name `%.*s` defined multiple times",
    //             TEXT_FMT(parameter->name));
    //         error = true;
    //     }
    // }
    // type_t return_type;
    // if (function->signature.has_return_type) {
    //     const symbol_t* type_symbol = find_symbol_of(
    //         *analyzer->current_scope,
    //         STRING_VIEW(function->signature.return_type.element_type_name),
    //         SYMBOL_TYPE
    //     );
    //     if (type_symbol == NULL) {
    //         // FIXME: where error?
    //         report_error("function return type specifier must be a type");
    //         error = true;
    //     } else {
    //         function->signature.return_type.type.element_type = type_symbol->as.type;
    //         return_type = function->signature.return_type.type;
    //     }
    // } else {
    //     return_type = (type_t){ .array_depth = 0, .element_type.kind = TYPE_UNIT };
    // }

    // if (error) {
    //     return ANALYZE_ERROR;
    // }

    // return analyze_block(analyzer, &function->body, &return_type, NULL);
}

void debug_function(function_t function, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "function");
    
    ast_debug_key(debugger, "signature");
    debug_function_signature(function.signature, debugger);

    ast_debug_key(debugger, "body");
    debug_block(function.body, debugger);

    ast_debug_end(debugger);
}
