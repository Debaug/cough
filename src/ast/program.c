#include <string.h>

#include "diagnostic/diagnostic.h"
#include "ast/program.h"
#include "ast/debug.h"

parse_result_t parse_item_declaration(parser_t* parser, item_declaration_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_t name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &name)) {
        PARSER_ERROR_RESTORE(parser, state);
    }
    dst->name = name.text;

    if (!match_parser(parser, TOKEN_COLON_COLON, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    parse_result_t result;
    switch (peek_parser(*parser).type) {
    case TOKEN_FN:
        dst->kind = ITEM_FUNCTION;
        result = parse_function(parser, &dst->as.function);
        break;
    case TOKEN_STRUCT:
        dst->kind = ITEM_STRUCT;
        result = parse_struct(parser, &dst->as.composite);
        break;
    case TOKEN_VARIANT:
        dst->kind = ITEM_VARIANT;
        result = parse_variant(parser, &dst->as.composite);
        break;
    default: result = PARSE_ERROR;
    }

    if (result != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    return PARSE_SUCCESS;
}

parse_result_t parse_program(parser_t* parser, program_t* program) {
    item_declaration_array_buf_t item_declarations = new_array_buf(item_declaration_t);
    while (!parser_is_eof(*parser)) {
        item_declaration_t item_declaration;
        if (parse_item_declaration(parser, &item_declaration) != PARSE_SUCCESS) {
            free_array_buf(item_declarations);
            return PARSE_ERROR;
        }
        array_buf_push(&item_declarations, item_declaration);
    }
    ast_push_alloc(&parser->storage, item_declarations.data);
    *program = (program_t){ .item_declarations = item_declarations };
    return PARSE_SUCCESS;
}

typedef struct function_item {
    text_view_t name;
    function_t* function;
} function_item_t;
typedef array_buf_t(function_item_t) function_item_array_buf_t;

analyze_result_t analyze_unordered_symbols(
    analyzer_t* analyzer,
    program_t* program
) {
    for (size_t i = 0; i < program->item_declarations.len; i++) {
        item_declaration_t* item = &program->item_declarations.data[i];
        element_type_kind_t type_kind;
        switch (item->kind) {
        case ITEM_FUNCTION:;
            function_item_t function_item = { .name = item->name, .function = &item->as.function };
            break;
        case ITEM_STRUCT: type_kind = TYPE_STRUCT; break;
        case ITEM_VARIANT: type_kind = TYPE_VARIANT; break;
        }
        symbol_t symbol = {
            .name = item->name,
            .kind = SYMBOL_TYPE,
            .as.type = (element_type_t){
                .kind = type_kind,
                .as.composite = &item->as.composite
            }
        };
        if (!add_symbol(analyzer->current_scope, symbol)) {
            // where error?
            report_error("type with name `%.*s` defined multiple times",
                TEXT_FMT(symbol.name));
        }
    }

    for (size_t i = 0; i < program->item_declarations.len; i++) {
        item_declaration_t* item = &program->item_declarations.data[i];
        switch (item->kind) {
        case ITEM_FUNCTION:;
            symbol_t symbol = {
                .name = item->name,
                .kind = SYMBOL_FUNCTION,
                .as.function = &item->as.function
            };
            if (!add_symbol(analyzer->current_scope, symbol)) {
                report_error("item with name `%.*s` defined multiple times",
                    TEXT_FMT(symbol.name));
            }
            function_signature_t* signature = &item->as.function.signature;
            if (signature->has_return_type) {
                exit(-1); // TODO
            }
            break;

        case ITEM_STRUCT:
        case ITEM_VARIANT:
            analyze_composite(analyzer, &item->as.composite);
            break;
        }
    }

    return ANALYZE_SUCCESS;
}

analyze_result_t analyze_expressions(analyzer_t* analyzer, program_t* program) {
    for (size_t i = 0; i < program->item_declarations.len; i++) {
        item_declaration_t* declaration = &program->item_declarations.data[i];
        if (declaration->kind != ITEM_FUNCTION) {
            continue;
        }
        analyze_function(analyzer, &declaration->as.function);
    }
    return ANALYZE_SUCCESS;
}

void debug_item_declaration(
    item_declaration_t item_declaration,
    ast_debugger_t* debugger
) {
    ast_debug_start(debugger, "item_declaration");

    ast_debug_key(debugger, "name");
    ast_debug_string_view(debugger, STRING_VIEW(item_declaration.name));

#define DEBUG_ITEM_DECLARATION_CASE(kind, dbg, name, str)   \
    case kind:                                              \
        ast_debug_key(debugger, str);                       \
        dbg (item_declaration.as.name, debugger);           \
        break;

    switch (item_declaration.kind) {
    DEBUG_ITEM_DECLARATION_CASE(ITEM_FUNCTION, debug_function, function, "function")
    DEBUG_ITEM_DECLARATION_CASE(ITEM_STRUCT, debug_struct, composite, "struct")
    DEBUG_ITEM_DECLARATION_CASE(ITEM_VARIANT, debug_variant, composite, "variant")
    }

    ast_debug_end(debugger);
}

void debug_program(
    program_t program,
    ast_debugger_t* debugger
) {
    ast_debug_start(debugger, "program");
    ast_debug_key(debugger, "item_declarations");
    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i < program.item_declarations.len; i++) {
        debug_item_declaration(
            program.item_declarations.data[i],
            debugger
        );
    }
    ast_debug_end_sequence(debugger);
    ast_debug_end(debugger);
}
