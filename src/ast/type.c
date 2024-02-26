#include "ast/type.h"

parse_result_t parse_type_name(parser_t* parser, type_name_t* dst) {
    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        return PARSE_ERROR;
    }

    *dst = (type_name_t){ .kind = TYPE_NAME_IDENTIFIER, .as = identifier.text };
    return PARSE_SUCCESS;
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
