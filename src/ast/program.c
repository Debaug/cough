#include "ast/program.h"
#include "ast/debug.h"

parse_item_declaration_result_t parse_item_declaration(parser_t* super_parser) {
    parser_t parser = *super_parser;

    token_t identifier;
    if (!match_parser(&parser, TOKEN_IDENTIFIER, &identifier)) {
        return RESULT_ERROR(parse_item_declaration_result_t, PARSE_ERROR);
    }

    if (!match_parser(&parser, TOKEN_COLON_COLON, NULL)) {
        return RESULT_ERROR(parse_item_declaration_result_t, PARSE_ERROR);
    }

    parse_function_result_t function = parse_function(&parser);
    if (function.is_ok) {
        *super_parser = parser;
        item_declaration_t item_declaration = {
            .identifier = identifier.text,
            .kind = FUNCTION_DECLARATION,
            .as = function.ok,
        };
        return RESULT_OK(parse_item_declaration_result_t, item_declaration);
    }

    return RESULT_ERROR(parse_item_declaration_result_t, PARSE_ERROR);
}

parse_program_result_t parse_program(parser_t* parser) {
    array_buf_t item_declarations = new_array_buf();
    while (!parser_is_eof(*parser)) {
        parse_item_declaration_result_t item_declaration = parse_item_declaration(parser);
        if (!item_declaration.is_ok) {
            return RESULT_ERROR(parse_program_result_t, PARSE_ERROR);
        }
        array_buf_push(&item_declarations, &item_declaration.ok, sizeof(item_declaration.ok));
    }
    program_t program = { .item_declarations = item_declarations };
    return RESULT_OK(parse_program_result_t, program);
}

void debug_item_declaration(item_declaration_t item_declaration, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "item_declaration");

    ast_debug_key(debugger, "identifier");
    ast_debug_string_view(debugger, STRING_VIEW(item_declaration.identifier));

    ast_debug_key(debugger, "function");
    debug_function(item_declaration.as.function, debugger);

    ast_debug_end(debugger);
}

void debug_program(program_t program, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "program");
    ast_debug_key(debugger, "item_declarations");
    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i * sizeof(item_declaration_t) < program.item_declarations.len; i++) {
        debug_item_declaration(((item_declaration_t*)program.item_declarations.ptr)[i], debugger);
    }
    ast_debug_end_sequence(debugger);
    ast_debug_end(debugger);
}
