#include "ast/function.h"

parse_parameter_result_t parse_parameter(parser_t* super_parser) {
    parser_t parser = *super_parser;

    token_t identifier;
    if (!match_parser(&parser, TOKEN_IDENTIFIER, &identifier)) {
        return RESULT_ERROR(parse_parameter_result_t, PARSE_ERROR);
    }

    if (!match_parser(&parser, TOKEN_COLON, NULL)) {
        return RESULT_ERROR(parse_parameter_result_t, PARSE_ERROR);
    }

    parse_type_name_result_t type_name = parse_type_name(&parser);
    if (!type_name.is_ok) {
        return CAST_ERROR(parse_parameter_result_t, type_name);
    }
    
    match_parser(&parser, TOKEN_COMMA, NULL);

    *super_parser = parser;
    named_type_t type = NAMED_TYPE_UNRESOLVED(type_name.ok);
    parameter_t parameter = { .identifier = identifier.text, .type = type };
    return RESULT_OK(parse_parameter_result_t, parameter);
}

parse_function_result_t parse_function(parser_t* super_parser) {
    parser_t parser = *super_parser;

    token_type_t start_pattern[2] = { TOKEN_FN, TOKEN_LEFT_PAREN };
    if (match_parser_sequence(&parser, start_pattern, NULL, 2) != 2) {
        return RESULT_ERROR(parse_function_result_t, PARSE_ERROR);
    }

    array_buf_t parameters = new_array_buf();
    while (!match_parser(&parser, TOKEN_RIGHT_PAREN, NULL)) {
        parse_parameter_result_t parameter = parse_parameter(&parser);
        if (!parameter.is_ok) {
            return CAST_ERROR(parse_function_result_t, parameter);
        }
        array_buf_push(&parameters, &parameter.ok, sizeof(parameter.ok));
    }

    bool has_return_type = match_parser(&parser, TOKEN_RIGHT_ARROW, NULL);
    named_type_t return_type = {0};
    if (has_return_type) {
        parse_type_name_result_t result = parse_type_name(&parser);
        if (!result.is_ok) {
            return CAST_ERROR(parse_function_result_t, result);
        }
        return_type = NAMED_TYPE_UNRESOLVED(result.ok);
    }

    parse_block_result_t body = parse_block(&parser);
    if (!body.is_ok) {
        return CAST_ERROR(parse_function_result_t, body);
    }

    function_t function = {
        .parameters = parameters,
        .has_return_type = has_return_type,
        .return_type = return_type,
        .body = body.ok
    };

    *super_parser = parser;

    return RESULT_OK(parse_function_result_t, function);
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
    for (size_t i = 0; i * sizeof(parameter_t) < function.parameters.len; i++) {
        debug_parameter(((parameter_t*)function.parameters.ptr)[i], debugger);
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
