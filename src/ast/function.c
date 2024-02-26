#include "ast/function.h"

parse_error_t parse_parameter(parser_t* parser, parameter_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    if (!match_parser(parser, TOKEN_COLON, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    type_name_t type_name;
    if (parse_type_name(parser, &type_name) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    named_type_t type = NAMED_TYPE_UNRESOLVED(type_name);
    *dst = (parameter_t){ .identifier = identifier.text, .type = type };
    return PARSE_SUCCESS;
}

parse_error_t parse_function(parser_t* parser, function_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_type_t start_pattern[2] = { TOKEN_FN, TOKEN_LEFT_PAREN };
    if (match_parser_sequence(parser, start_pattern, NULL, 2) != 2) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    parameter_array_buf_t parameters = new_array_buf(parameter_t);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        parameter_t parameter;
        if (parse_parameter(parser, &parameter) != PARSE_SUCCESS) {
            free_array_buf(parameters);
            PARSER_ERROR_RESTORE(parser, state);
        }
        array_buf_push(&parameters, parameter);

        if (!match_parser(parser, TOKEN_COMMA, NULL)) {
            if (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
                free_array_buf(parameters);
                PARSER_ERROR_RESTORE(parser, state);
            }
            break;
        }
    }
    alloc_stack_push(&parser->storage.allocations, parameters.data);

    bool has_return_type = match_parser(parser, TOKEN_RIGHT_ARROW, NULL);
    named_type_t return_type = {0};
    if (has_return_type) {
        type_name_t return_type_name;
        if (parse_type_name(parser, &return_type_name) != PARSE_SUCCESS) {
            PARSER_ERROR_RESTORE(parser, state);
        }
    }

    block_t body;
    if (parse_block(parser, &body) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    *dst = (function_t){
        .parameters = parameters,
        .has_return_type = has_return_type,
        .return_type = return_type,
        .body = body
    };
    return PARSE_SUCCESS;
}

void debug_parameter(parameter_t parameter, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "parameter");

    ast_debug_key(debugger, "identifier");
    ast_debug_string_view(debugger, STRING_VIEW(parameter.identifier));

    ast_debug_key(debugger, "type");
    debug_named_type(parameter.type, debugger);

    ast_debug_end(debugger);
}

void debug_function(function_t function, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "function");
    ast_debug_key(debugger, "parameters");

    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i < function.parameters.len; i++) {
        debug_parameter(function.parameters.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);

    if (function.has_return_type) {
        ast_debug_key(debugger, "return_type");
        debug_named_type(function.return_type, debugger);
    }

    ast_debug_key(debugger, "body");
    debug_block(function.body, debugger);

    ast_debug_end(debugger);
}
