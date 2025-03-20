#include "compiler/ast/function.h"

Result parse_function(Parser* parser, Function* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);

    FunctionSignature signature;
    if (parse_function_signature(parser, &signature) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Block body;
    if (parse_block(parser, &body) != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    *dst = (Function){
        .signature = signature,
        .body = body
    };
    return SUCCESS;
}

Result analyze_function(Analyzer* analyzer, Function* function) {
    eprintf("TODO: analyze_function\n");
    exit(EXIT_FAILURE);
    return SUCCESS;

    // scope_t* scope = analyzer_enter_new_scope(analyzer);

    // bool error = false;
    // for (usize i = 0; i < function->signature.parameters.len; i++) {
    //     Variable* parameter = &function->signature.parameters.data[i];
    //     const Symbol* type_symbol = find_symbol_of(
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
        
    //     Symbol parameter_symbol = {
    //         .name = parameter->name,
    //         .kind = SYMBOL_VARIABLE,
    //         .as.variable = parameter
    //     };
    //     if (!add_symbol(scope, parameter_symbol)) {
    //         // FIXME: where error?
    //         report_error("parameter with name `%.*s` defined multiple times",
    //             STRING_FMT(parameter->name));
    //         error = true;
    //     }
    // }
    // Type return_type;
    // if (function->signature.has_return_type) {
    //     const Symbol* type_symbol = find_symbol_of(
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
    //     return_type = (Type){ .array_depth = 0, .element_type.kind = TYPE_UNIT };
    // }

    // if (error) {
    //     return ANALYZE_ERROR;
    // }

    // return analyze_block(analyzer, &function->body, &return_type, NULL);
}

void debug_function(Function function, AstDebugger* debugger) {
    ast_debug_start(debugger, "function");
    
    ast_debug_key(debugger, "signature");
    debug_function_signature(function.signature, debugger);

    ast_debug_key(debugger, "body");
    debug_block(function.body, debugger);

    ast_debug_end(debugger);
}
