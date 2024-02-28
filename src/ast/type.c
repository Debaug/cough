#include "ast/type.h"

parse_result_t parse_type_name(parser_t* parser, named_type_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    size_t array_depth = 0;
    while (match_parser(parser, TOKEN_LEFT_BRACKET, NULL)) {
        array_depth++;
    }

    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    for (size_t i = 0; i < array_depth; i++) {
        if (!match_parser(parser, TOKEN_RIGHT_BRACKET, &identifier)) {
            PARSER_ERROR_RESTORE(parser, state);
        }
    }

    *dst = (named_type_t){
        .array_depth = array_depth,
        .element_type_name = identifier.text,
        .element_type = (element_type_t){ .kind = TYPE_NEVER },
    };
    return PARSE_SUCCESS;
}

parse_result_t parse_variable(parser_t* parser, variable_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_t identifier;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &identifier)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    if (!match_parser(parser, TOKEN_COLON, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    named_type_t type;
    if (parse_type_name(parser, &type) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    *dst = (variable_t){ .name = identifier.text, .type = type };
    return PARSE_SUCCESS;
}

static parse_result_t parse_composite(parser_t* parser, composite_type_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    
    // skip leading `struct` or `enum` token
    step_parser(parser);

    if (!match_parser(parser, TOKEN_LEFT_BRACE, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    variable_array_buf_t fields = new_array_buf();
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        variable_t field;
        if (parse_variable(parser, &field) != PARSE_SUCCESS) {
            free_array_buf(fields);
            PARSER_ERROR_RESTORE(parser, state);
        }
        array_buf_push(&fields, field);
        if (match_parser(parser, TOKEN_COMMA, NULL)) {
            continue;
        }
        if (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
            free_array_buf(fields);
            PARSER_ERROR_RESTORE(parser, state);
        }
        break;
    }

    ast_box(&parser->storage, fields);
    *dst = (composite_type_t){ .fields = fields };
    return PARSE_SUCCESS;
}

parse_result_t parse_struct(parser_t* parser, composite_type_t* dst) {
    return parse_composite(parser, dst);
}

parse_result_t parse_variant(parser_t* parser, composite_type_t* dst) {
    return parse_composite(parser, dst);
}

void debug_named_type(named_type_t type, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "named_type");

    ast_debug_key(debugger, "array_depth");
    ast_debug_uint(debugger, type.array_depth);

    ast_debug_key(debugger, "element_type");
    switch (type.element_type.kind) {
    case TYPE_NEVER: ast_debug_string(debugger, "never"); break;
    case TYPE_UNIT: ast_debug_string(debugger, "unit"); break;
    case TYPE_BOOL: ast_debug_string(debugger, "bool"); break;
    case TYPE_INT: ast_debug_string(debugger, "int"); break;
    case TYPE_FLOAT: ast_debug_string(debugger, "float"); break;
    case TYPE_STRUCT:
        debug_struct(*type.element_type.as.composite, debugger);
        break;
    case TYPE_VARIANT:
        debug_variant(*type.element_type.as.composite, debugger);
        break;
    }

    ast_debug_end(debugger);
}

void debug_variable(variable_t variable, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "variable");

    if (variable.mutable) {
        ast_debug_flag(debugger, "mutable");
    }

    ast_debug_key(debugger, "name");
    ast_debug_string_view(debugger, STRING_VIEW(variable.name));

    ast_debug_key(debugger, "type");
    debug_named_type(variable.type, debugger);

    ast_debug_end(debugger);
}

static void debug_composite_type(composite_type_t type, const char* kind, ast_debugger_t* debugger) {
    ast_debug_start(debugger, kind);

    ast_debug_key(debugger, "fields");
    ast_debug_start_sequence(debugger);
    for (int i = 0; i < type.fields.len; i++) {
        ast_debug_start(debugger, "field");

        ast_debug_key(debugger, "name");
        ast_debug_string_view(debugger, STRING_VIEW(type.fields.data[i].name));

        ast_debug_key(debugger, "type");
        debug_named_type(type.fields.data[i].type, debugger);
        
        ast_debug_end(debugger);
    }
    ast_debug_end_sequence(debugger);
    ast_debug_end(debugger);
}

void debug_struct(composite_type_t struct_, ast_debugger_t* debugger) {
    debug_composite_type(struct_, "struct", debugger);
}

void debug_variant(composite_type_t variant, ast_debugger_t* debugger) {
    debug_composite_type(variant, "variant", debugger);
}
