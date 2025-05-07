#include "compiler/diagnostics.h"
#include "compiler/ast/type.h"
#include "compiler/ast/analyzer.h"

bool type_eq(Type a, Type b) {
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

bool function_signature_eq(FunctionSignature a, FunctionSignature b) {
    if (a.parameters.len != b.parameters.len) {
        return false;
    }
    for (usize i = 0; i < a.parameters.len; i++) {
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

Result parse_type_name(Parser* parser, NamedType* dst) {
    // TODO: array type

    Token type_name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &type_name)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_INVALID_TYPE_NAME,
            format("invalid type name (expected identifier)"),
            peek_parser(*parser).text
        );
        return ERROR;
    }
    *dst = (NamedType){
        .type = {
            .array_depth = 0,
            .element_type = TYPE_NEVER, // will get resolved during analysis
        },
        .element_type_name = type_name.text
    };
    return SUCCESS;
}

static Result parse_field_or_variable_tail(Parser* parser, Field* dst) {
    // TODO: better error message depending on field or variable

    Token name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &name)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_VARIABLE_DECLARATION_INVALID_NAME,
            format("invalid variable or field name (expected identifier)"),
            peek_parser(*parser).text
        );
        return ERROR;
    }

    if (!match_parser(parser, TOKEN_COLON, NULL)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_VARIABLE_DECLARATION_INVALID_TYPE,
            format("missing type specification in variable or field declaration (expected `: <type>`)"),
            peek_parser(*parser).text
        );
        return ERROR;
    }

    NamedType type;
    if (parse_type_name(parser, &type) != SUCCESS) {
        // error is reported in `parse_type_name`
        // we do not return `SUCCESS` as recovery is handled upstream
        return ERROR;
    }
    
    // size and offset are determined during analysis
    *dst = (Field){ .name = name.text, .type = type };
    return SUCCESS;
}

Result parse_variable(Parser* parser, Variable* dst) {
    bool mutable = match_parser(parser, TOKEN_MUT, NULL);
    Field as_field;
    if (parse_field_or_variable_tail(parser, &as_field) != SUCCESS) {
        return ERROR;
    }

    // size and offset are determined during analysis
    *dst = (Variable){
        .mutable = mutable,
        .name = as_field.name,
        .type = as_field.type,
    };
    return SUCCESS;
}

Result parse_function_signature(Parser* parser, FunctionSignature* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);

    TokenKind start_pattern[2] = { TOKEN_FN, TOKEN_LEFT_PAREN };
    usize nmatching_start = match_parser_sequence(parser, start_pattern, NULL, 2);
    if (nmatching_start != 2) {
        report_simple_compiler_error(
            parser->reporter,
            CE_NOT_FUNCTION_SIGNATURE,
            format("invalid function signature (expected `fn(...)`)"),
            peek_parser_nth(*parser, nmatching_start).text
        );
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    VariableArrayBuf parameters = new_array_buf(Variable);
    while (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
        Variable parameter;
        if (parse_variable(parser, &parameter) != SUCCESS) {
            TokenKind skip_until[2] = { TOKEN_COMMA, TOKEN_RIGHT_PAREN };
            skip_parser_until_any_of(parser, skip_until, 2);
            continue;
        } else {
            array_buf_push(&parameters, parameter);
        }

        if (!match_parser(parser, TOKEN_COMMA, NULL)) {
            if (!match_parser(parser, TOKEN_RIGHT_PAREN, NULL)) {
                free_array_buf(parameters);

                report_simple_compiler_error(
                    parser->reporter,
                    CE_UNCLOSED_PARENS,
                    format("expected right parenthesis `)` after parameter list"),
                    peek_parser(*parser).text
                );
                parser_restore_alloc(parser, state);
                return ERROR;
            }
            break;
        }
    }

    bool has_return_type = match_parser(parser, TOKEN_RIGHT_ARROW, NULL);
    NamedType return_type = {0};
    if (has_return_type) {
        if (parse_type_name(parser, &return_type) != SUCCESS) {
            parser_restore_alloc(parser, state);
            return ERROR;
        }
    }

    ast_push_alloc(&parser->storage, parameters.data);
    *dst = (FunctionSignature){
        .parameters = parameters,
        .has_return_type = has_return_type,
        .return_type = return_type,
    };
    return SUCCESS;
}

static Result parse_composite(Parser* parser, CompositeType* dst) {
    // skip leading `struct` or `variant` token
    step_parser(parser);

    if (!match_parser(parser, TOKEN_LEFT_BRACE, NULL)) {
        report_simple_compiler_error(
            parser->reporter,
            CE_COMPOSITE_MISSING_FIELD_LIST,
            format("missing fields of struct or variant type declaration (expected `{ ... }`)"),
            peek_parser(*parser).text
        );

        return ERROR;
    }
    
    ParserAllocState state = parser_snapshot_alloc(*parser);
    
    FieldArrayBuf fields = new_array_buf();
    while (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
        Field field;
        if (parse_field_or_variable_tail(parser, &field) != SUCCESS) {
            TokenKind skip_until[2] = { TOKEN_RIGHT_BRACE, TOKEN_COMMA };
            skip_parser_until_any_of(parser, skip_until, 2);
        } else {
            array_buf_push(&fields, field);
        }
        if (match_parser(parser, TOKEN_COMMA, NULL)) {
            continue;
        }
        if (!match_parser(parser, TOKEN_RIGHT_BRACE, NULL)) {
            free_array_buf(fields);
            
            report_simple_compiler_error(
                parser->reporter,
                CE_UNCLOSED_BRACES,
                format("expected right brace `}` after field list"),
                peek_parser(*parser).text
            );
            parser_restore_alloc(parser, state);
            return ERROR;
        }
        break;
    }

    ast_push_alloc(&parser->storage, fields.data);
    *dst = (CompositeType){ .fields = fields };
    return SUCCESS;
}

Result parse_struct(Parser* parser, CompositeType* dst) {
    return parse_composite(parser, dst);
}

Result parse_variant(Parser* parser, CompositeType* dst) {
    return parse_composite(parser, dst);
}

Result analyze_type(Analyzer* analyzer, NamedType* type) {
    eprintf("todo `analyze_type`\n");
    abort();
    return SUCCESS;
}

Result analyze_function_signature(
    Analyzer* analyzer,
    FunctionSignature* signature
) {
    eprintf("todo `analyze_function_signature`\n");
    abort();
    return SUCCESS;
}

const Field* find_field(CompositeType type, StringView name) {
    for (usize i = 0; i < type.fields.len; i++) {
        if (string_views_eq(STRING_VIEW(type.fields.data[i].name), name)) {
            return &type.fields.data[i];
        }
    }
    return NULL;
}

void debug_function_signature(FunctionSignature signature, AstDebugger* debugger) {
    ast_debug_start(debugger, "function_signature");

    ast_debug_key(debugger, "parameters");
    ast_debug_start_sequence(debugger);
    for (usize i = 0; i < signature.parameters.len; i++) {
        debug_variable(signature.parameters.data[i], debugger);
    }
    ast_debug_end_sequence(debugger);

    if (signature.has_return_type) {
        ast_debug_key(debugger, "return_type");
        debug_named_type(signature.return_type, debugger);
    }

    ast_debug_end(debugger);
}

static void debug_element_type(ElementType type, AstDebugger* debugger) {
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

void debug_type(Type type, AstDebugger* debugger) {
    ast_debug_start(debugger, "type");

    ast_debug_key(debugger, "array_depth");
    ast_debug_uint(debugger, type.array_depth);

    ast_debug_key(debugger, "element_type");
    debug_element_type(type.element_type, debugger);

    ast_debug_end(debugger);
}

void debug_named_type(NamedType type, AstDebugger* debugger) {
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
    CompositeType type,
    const char* kind,
    AstDebugger* debugger
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

void debug_struct(CompositeType struct_, AstDebugger* debugger) {
    debug_composite_type(struct_, "struct", debugger);
}

void debug_variant(CompositeType variant, AstDebugger* debugger) {
    debug_composite_type(variant, "variant", debugger);
}

void debug_field(Field field, AstDebugger* debugger) {
    ast_debug_start(debugger, "field");

    ast_debug_key(debugger, "name");
    ast_debug_string_view(debugger, STRING_VIEW(field.name));

    ast_debug_key(debugger, "type");
    debug_named_type(field.type, debugger);

    ast_debug_end(debugger);
}

void debug_variable(Variable variable, AstDebugger* debugger) {
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
