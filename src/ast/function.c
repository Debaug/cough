#include "ast/function.h"

parse_result_t parse_function(parser_t* parser, function_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_type_t start_pattern[2] = { TOKEN_FN, TOKEN_LEFT_PAREN };
    if (match_parser_sequence(parser, start_pattern, NULL, 2) != 2) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    variable_array_buf_t parameters = new_array_buf(variable_t);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        variable_t parameter;
        if (parse_variable(parser, &parameter) != PARSE_SUCCESS) {
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
    ast_push_alloc(&parser->storage, parameters.data);

    bool has_return_type = match_parser(parser, TOKEN_RIGHT_ARROW, NULL);
    named_type_t return_type = {0};
    if (has_return_type) {
        if (parse_type_name(parser, &return_type) != PARSE_SUCCESS) {
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

void debug_function(function_t function, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "function");
    ast_debug_key(debugger, "parameters");

    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i < function.parameters.len; i++) {
        debug_variable(function.parameters.data[i], debugger);
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
