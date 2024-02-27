#include "ast/program.h"
#include "ast/debug.h"

parse_result_t parse_item_declaration(parser_t* parser, item_declaration_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    if (!match_parser(parser, TOKEN_COLON_COLON, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    function_t function;
    if (parse_function(parser, &function) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }
    *dst = (item_declaration_t){
        .identifier = identifier.text,
        .kind = FUNCTION_DECLARATION,
        .as.function = function,
    };
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

void debug_item_declaration(
    item_declaration_t item_declaration,
    ast_debugger_t* debugger
) {
    ast_debug_start(debugger, "item_declaration");

    ast_debug_key(debugger, "identifier");
    ast_debug_string_view(debugger, STRING_VIEW(item_declaration.identifier));

    ast_debug_key(debugger, "function");
    debug_function(item_declaration.as.function, debugger);

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
