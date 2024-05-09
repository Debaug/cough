#include "ast/type.h"
#include "ast/analyzer.h"

bool type_eq(type_t a, type_t b) {
    if (a.array_depth != b.array_depth) {
        return false;
    }
    if (a.element_type.kind != b.element_type.kind) {
        return false;
    }
    if (a.element_type.kind == TYPE_FUNCTION) {
        return function_signature_eq(
            *a.element_type.as.function_signature,
            *b.element_type.as.function_signature
        );
    }
    switch (a.element_type.kind) {
    case TYPE_NEVER:
    case TYPE_UNIT:
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_FLOAT:
        return a.element_type.kind == b.element_type.kind;
    case TYPE_FUNCTION:

    case TYPE_STRUCT:
    case TYPE_VARIANT:
        return a.element_type.as.composite == b.element_type.as.composite;
    }
}

bool function_signature_eq(function_signature_t a, function_signature_t b) {
    if (a.parameters.len != b.parameters.len) {
        return false;
    }
    for (size_t i = 0; i < a.parameters.len; i++) {
        if (!type_eq(a.parameters.data[i].type.type, b.parameters.data[i].type.type)) {
            return false;
        }
    }

    bool return_unit;
    if (!a.has_return_type) {
        return_unit = true;
    } else if (
        a.return_type.type.array_depth == 0
        && a.return_type.type.element_type.kind == TYPE_UNIT
    ) {
        return_unit = true;
    } else {
        return_unit = false;
    }

    if (return_unit) {
        if (!b.has_return_type) {
            return true;
        } else if (
            b.return_type.type.array_depth == 0
            && b.return_type.type.element_type.kind == TYPE_UNIT
        ) {
            return true;
        } else {
            return false;
        }
    }

    return type_eq(a.return_type.type, b.return_type.type);
}

parse_result_t parse_type_name(parser_t* parser, named_type_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    size_t array_depth = 0;
    while (match_parser(parser, TOKEN_LEFT_BRACKET, NULL)) {
        array_depth++;
    }

    token_t name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &name)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    for (size_t i = 0; i < array_depth; i++) {
        if (!match_parser(parser, TOKEN_RIGHT_BRACKET, NULL)) {
            PARSER_ERROR_RESTORE(parser, state);
        }
    }

    *dst = (named_type_t){
        .type.array_depth = array_depth,
        .type.element_type = (element_type_t){ .kind = TYPE_NEVER },
        .element_type_name = name.text,
    };
    return PARSE_SUCCESS;
}

static parse_result_t parse_field(parser_t* parser, field_t* dst) {
    parser_state_t state = parser_snapshot(*parser);

    token_t name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &name)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    if (!match_parser(parser, TOKEN_COLON, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    named_type_t type;
    if (parse_type_name(parser, &type) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    *dst = (field_t){ .name = name.text, .type = type };
    return PARSE_SUCCESS;
}

parse_result_t parse_variable(parser_t* parser, variable_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    bool mutable = match_parser(parser, TOKEN_MUT, NULL);
    field_t as_field;
    if (parse_field(parser, &as_field) != PARSE_SUCCESS) {
        PARSER_ERROR_RESTORE(parser, state);
    }
    *dst = (variable_t){
        .mutable = mutable,
        .name = as_field.name,
        .type = as_field.type,
        .offset = as_field.offset,
        .size = as_field.offset,
    };
    return PARSE_SUCCESS;
}

parse_result_t parse_function_signature(parser_t* parser, function_signature_t* dst) {
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

    *dst = (function_signature_t){
        .parameters = parameters,
        .has_return_type = has_return_type,
        .return_type = return_type,
    };
    return PARSE_SUCCESS;
}

static parse_result_t parse_composite(parser_t* parser, composite_type_t* dst) {
    parser_state_t state = parser_snapshot(*parser);
    
    // skip leading `struct` or `enum` token
    step_parser(parser);

    if (!match_parser(parser, TOKEN_LEFT_BRACE, NULL)) {
        PARSER_ERROR_RESTORE(parser, state);
    }

    field_array_buf_t fields = new_array_buf();
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        field_t field;
        if (parse_field(parser, &field) != PARSE_SUCCESS) {
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

analyze_result_t analyze_type(analyzer_t* analyzer, named_type_t* type) {
    switch (type->type.element_type.kind) {
    case TYPE_NEVER:
    case TYPE_UNIT:
    case TYPE_BOOL:
    case TYPE_INT:
    case TYPE_FLOAT:
        break;
    case TYPE_FUNCTION:
        analyze_function_signature(analyzer, type->type.element_type.as.)
    }
}

analyze_result_t analyze_function_signature(
    analyzer_t* analyzer,
    function_signature_t* signature
) {
    analyze_result_t result;
    if (signature->has_return_type) {
        switch (signature->return_type.type.element_type.kind) {
            case TYPE_
        }
    }
    return result;
}

const field_t* find_field(composite_type_t type, string_view_t name) {
    for (size_t i = 0; i < type.fields.len; i++) {
        if (string_views_eq(STRING_VIEW(type.fields.data[i].name), name)) {
            return &type.fields.data[i];
        }
    }
    return NULL;
}

void debug_function_signature(function_signature_t signature, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "function_signature");

    ast_debug_key(debugger, "parameters");
    ast_debug_start_sequence(debugger);
    for (size_t i = 0; i < signature.parameters.len; i++) {
        debug_variable(signature.parameters.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);

    if (signature.has_return_type) {
        ast_debug_key(debugger, "return_type");
        debug_type(signature.return_type.type, debugger);
    }

    ast_debug_end(debugger);
}

void debug_type(type_t type, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "named_type");

    ast_debug_key(debugger, "array_depth");
    ast_debug_uint(debugger, type.array_depth);

    ast_debug_key(debugger, "element_type");
    switch (type.element_type.kind) {
    case TYPE_NEVER: ast_debug_string(debugger, "Never"); break;
    case TYPE_UNIT: ast_debug_string(debugger, "Unit"); break;
    case TYPE_BOOL: ast_debug_string(debugger, "Bool"); break;
    case TYPE_INT: ast_debug_string(debugger, "Int"); break;
    case TYPE_FLOAT: ast_debug_string(debugger, "Float"); break;
    case TYPE_FUNCTION:
        debug_function_signature(
            *type.element_type.as.function_signature,
            debugger
        );
        break;
    case TYPE_STRUCT:
        debug_struct(*type.element_type.as.composite, debugger);
        break;
    case TYPE_VARIANT:
        debug_variant(*type.element_type.as.composite, debugger);
        break;
    }

    ast_debug_end(debugger);
}

static void debug_composite_type(
    composite_type_t type,
    const char* kind,
    ast_debugger_t* debugger
) {
    ast_debug_start(debugger, kind);

    ast_debug_key(debugger, "fields");
    ast_debug_start_sequence(debugger);
    for (int i = 0; i < type.fields.len; i++) {
        debug_field(type.fields.data[i], debugger);
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

void debug_field(field_t field, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "field");

    ast_debug_key(debugger, "name");
    ast_debug_string_view(debugger, STRING_VIEW(field.name));

    ast_debug_key(debugger, "type");
    debug_type(field.type.type, debugger);

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
    debug_type(variable.type.type, debugger);

    ast_debug_end(debugger);
}
