#include "ast/type.h"

parse_type_name_result_t parse_type_name(parser_t* parser) {
    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        return RESULT_ERROR(parse_type_name_result_t, PARSE_ERROR);
    }

    type_name_t type_name = { .kind = TYPE_NAME_IDENTIFIER, .as = identifier.text };
    return RESULT_OK(parse_type_name_result_t, type_name);
}

void debug_type(type_t type, ast_debugger_t* debugger) {
    ast_debug_uint(debugger, type);
}

void debug_named_type(named_type_t type, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "named_type");

    ast_debug_key(debugger, "name");
    ast_debug_string_view(debugger, STRING_VIEW(type.name.as.identifier));

    ast_debug_key(debugger, "type");
    debug_type(type.type, debugger);

    ast_debug_end(debugger);
}
