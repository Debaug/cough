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

result_t parse_type_name(parser_t* parser, named_type_t* dst) {
    // TODO: array type

    token_t type_name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &type_name)) {
        error_t error = {
            .kind = ERROR_INVALID_TYPE_NAME,
            .message = format("invalid type name (expected identifier)"),
            .source = peek_parser(*parser).text,
        };
        report(parser->reporter, error);
        return ERROR;
    }
    *dst = (named_type_t){
        .type = {
            .array_depth = 0,
            .element_type = TYPE_NEVER, // will get resolved during analysis
        },
        .element_type_name = type_name.text
    };
    return SUCCESS;
}

static result_t parse_field_or_variable_tail(parser_t* parser, field_t* dst) {
    // TODO: better error message depending on field or variable

    token_t name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &name)) {
        error_t error = {
            .kind = ERROR_VARIABLE_DECLARATION_INVALID_NAME,
            .message = format("invalid variable or field name (expected identifier)"),
            .source = peek_parser(*parser).text,
        };
        report(parser->reporter, error);
        return ERROR;
    }

    if (!match_parser(parser, TOKEN_COLON, NULL)) {
        error_t error = {
            .kind = ERROR_VARIABLE_DECLARATION_INVALID_TYPE,
            .message = format("missing type specification in variable or field declaration (expected `: <type>`)"),
            .source = peek_parser(*parser).text,
        };
        report(parser->reporter, error);
        return ERROR;
    }

    named_type_t type;
    if (parse_type_name(parser, &type) != SUCCESS) {
        // error is reported in `parse_type_name`
        // we do not return `SUCCESS` as recovery is handled upstream
        return ERROR;
    }
    
    // size and offset are determined during analysis
    *dst = (field_t){ .name = name.text, .type = type };
    return SUCCESS;
}

result_t parse_variable(parser_t* parser, variable_t* dst) {
    bool mutable = match_parser(parser, TOKEN_MUT, NULL);
    field_t as_field;
    if (parse_field_or_variable_tail(parser, &as_field) != SUCCESS) {
        return ERROR;
    }

    // size and offset are determined during analysis
    *dst = (variable_t){
        .mutable = mutable,
        .name = as_field.name,
        .type = as_field.type,
    };
    return SUCCESS;
}

result_t parse_function_signature(parser_t* parser, function_signature_t* dst) {
    parser_alloc_state_t state = parser_snapshot(*parser);

    token_type_t start_pattern[2] = { TOKEN_FN, TOKEN_LEFT_PAREN };
    size_t nmatching_start = match_parser_sequence(parser, start_pattern, NULL, 2);
    if (nmatching_start != 2) {
        error_t error = {
            .kind = ERROR_NOT_FUNCTION_SIGNATURE,
            .message = format("invalid function signature (expected `fn (...)`)"),
            .source = peek_parser_nth(*parser, nmatching_start).text,
        };
        parser_error_restore(parser, state, error);
        return ERROR;
    }

    variable_array_buf_t parameters = new_array_buf(variable_t);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        variable_t parameter;
        if (parse_variable(parser, &parameter) != SUCCESS) {
            token_type_t skip_until[2] = { TOKEN_COMMA, TOKEN_RIGHT_PAREN };
            skip_parser_until_any_of(parser, skip_until, 2);
            continue;
        } else {
            array_buf_push(&parameters, parameter);
        }

        if (!match_parser(parser, TOKEN_COMMA, NULL)) {
            if (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
                free_array_buf(parameters);
                error_t error = {
                    .kind = ERROR_UNCLOSED_PARAMETER_LIST,
                    .message = format("expected right parenthesis `)` after parameter list"),
                    .source = peek_parser(*parser).text,
                };
                parser_error_restore(parser, state, error);
                return ERROR;
            }
            break;
        }
    }

    bool has_return_type = match_parser(parser, TOKEN_RIGHT_ARROW, NULL);
    named_type_t return_type = {0};
    if (has_return_type) {
        if (parse_type_name(parser, &return_type) != SUCCESS) {
            parser_restore(parser, state);
            return ERROR;
        }
    }

    ast_push_alloc(&parser->storage, parameters.data);
    *dst = (function_signature_t){
        .parameters = parameters,
        .has_return_type = has_return_type,
        .return_type = return_type,
    };
    return SUCCESS;
}

static result_t parse_composite(parser_t* parser, composite_type_t* dst) {
    // skip leading `struct` or `variant` token
    step_parser(parser);

    if (!match_parser(parser, TOKEN_LEFT_BRACE, NULL)) {
        error_t error = {
            .kind = ERROR_COMPOSITE_MISSING_FIELD_LIST,
            .message = format("missing fields of struct or variant type declaration (expected `{ ... }`)"),
            .source = peek_parser(*parser).text,
        };
        report(parser->reporter, error);
        return ERROR;
    }
    
    parser_alloc_state_t state = parser_snapshot(*parser);
    
    field_array_buf_t fields = new_array_buf();
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        field_t field;
        if (parse_field_or_variable_tail(parser, &field) != SUCCESS) {
            token_type_t skip_until[2] = { TOKEN_RIGHT_BRACE, TOKEN_COMMA };
            skip_parser_until_any_of(parser, skip_until, 2);
        } else {
            array_buf_push(&fields, field);
        }
        if (match_parser(parser, TOKEN_COMMA, NULL)) {
            continue;
        }
        if (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
            free_array_buf(fields);
            error_t error = {
                .kind = ERROR_UNCLOSED_PARAMETER_LIST,
                .message = format("expected right brace `}` after parameter list"),
                .source = peek_parser(*parser).text,
            };
            parser_error_restore(parser, state, error);
            return ERROR;
        }
        break;
    }

    ast_push_alloc(&parser->storage, fields.data);
    *dst = (composite_type_t){ .fields = fields };
    return SUCCESS;
}

result_t parse_struct(parser_t* parser, composite_type_t* dst) {
    return parse_composite(parser, dst);
}

result_t parse_variant(parser_t* parser, composite_type_t* dst) {
    return parse_composite(parser, dst);
}

result_t analyze_type(analyzer_t* analyzer, named_type_t* type) {
    eprintf("todo `analyze_type`\n");
    abort();
    return SUCCESS;
}

result_t analyze_function_signature(
    analyzer_t* analyzer,
    function_signature_t* signature
) {
    eprintf("todo `analyze_function_signature`\n");
    abort();
    return SUCCESS;
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
        debug_named_type(signature.return_type, debugger);
    }

    ast_debug_end(debugger);
}

static void debug_element_type(element_type_t type, ast_debugger_t* debugger) {
    switch (type.kind) {
        case TYPE_NEVER: ast_debug_string(debugger, "Never"); break;
        case TYPE_UNIT: ast_debug_string(debugger, "Unit"); break;
        case TYPE_BOOL: ast_debug_string(debugger, "Bool"); break;
        case TYPE_INT: ast_debug_string(debugger, "Int"); break;
        case TYPE_FLOAT: ast_debug_string(debugger, "Float"); break;
        case TYPE_FUNCTION:
            debug_function_signature(
                *type.as.function_signature,
                debugger
            );
            break;
        case TYPE_STRUCT:
            debug_struct(*type.as.composite, debugger);
            break;
        case TYPE_VARIANT:
            debug_variant(*type.as.composite, debugger);
            break;
        }
}

void debug_type(type_t type, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "type");

    ast_debug_key(debugger, "array_depth");
    ast_debug_uint(debugger, type.array_depth);

    ast_debug_key(debugger, "element_type");
    debug_element_type(type.element_type, debugger);

    ast_debug_end(debugger);
}

void debug_named_type(named_type_t type, ast_debugger_t* debugger) {
    ast_debug_start(debugger, "named_type");

    ast_debug_key(debugger, "array_depth");
    ast_debug_uint(debugger, type.type.array_depth);

    ast_debug_key(debugger, "element_type_name");
    ast_debug_string_view(debugger, STRING_VIEW(type.element_type_name));

    ast_debug_key(debugger, "element_type");
    debug_element_type(type.type.element_type, debugger);

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
    debug_named_type(field.type, debugger);

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
