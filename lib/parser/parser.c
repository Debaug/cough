#include "parser/parser.h"

typedef struct Parser {
    String source;
    TokenStream tokens;
    usize pos;
    Reporter* reporter;
    ArrayBuf(Expression) expressions;
    AstStorage storage;
} Parser;

static bool parser_match(Parser* parser, TokenKind kind, Token* dst) {
    ArrayBuf(Token) tokens = parser->tokens.tokens;
    if (tokens.len == parser->pos) {
        return false;
    }
    if (tokens.data[parser->pos].kind != kind) {
        return false;
    }
    if (dst) {
        *dst = tokens.data[parser->pos];
    }
    parser->pos++;
    return true;
}

static void parser_skip_until(Parser* parser, TokenKind kind) {
    TokenStream tokens = parser->tokens;
    usize paren_depth = 0;
    while (parser->pos != tokens.tokens.len) {
        Token token = tokens.tokens.data[parser->pos];
        if (paren_depth == 0 && token.kind == kind) {
            return;
        }
        if (token.kind == TOKEN_PAREN_LEFT) {
            paren_depth++;
        }
        if (token.kind == TOKEN_PAREN_RIGHT && paren_depth >= 0) {
            paren_depth--;
        }
        parser->pos++;
    }
}

static ExpressionId box_expression(Parser* parser, Expression expression) {
    usize id = parser->expressions.len;
    array_buf_push(Expression)(&parser->expressions, expression);
    return id;
}

static Result parse_module(Parser* parser, Module* dst);
static Result parse_constant(Parser* parser, ConstantDef* dst);
static Result parse_expression(Parser* parser, Expression* dst);
// head token must be `fn`
static Result parse_function(Parser* parser, Function* dst, Range* range);
static Result parse_pattern(Parser* parser, Pattern* dst);
static Result parse_type_name(Parser* parser, TypeName* dst);
static Result parse_identifier(Parser* parser, Identifier* dst);

bool parse(
    String source,
    TokenStream tokens,
    Reporter* reporter,
    Ast* dst
) {
    Parser parser = {
        .source = source,
        .tokens = tokens,
        .pos = 0,
        .reporter = reporter,
        .expressions = array_buf_new(Expression)(),
        .storage = ast_storage_new(),
    };
    Module module;
    parse_module(&parser, &module);
    ast_store(&parser.storage, parser.expressions.data);
    *dst = (Ast){
        .scopes = scope_graph_new(),
        .types = type_registry_new(),
        .expressions = parser.expressions,
        .root = module,
        .storage = parser.storage,
    };
    return reporter_error_count(reporter) == 0;
}

static Result parse_module(Parser* parser, Module* dst) {
    TokenStream tokens = parser->tokens;
    ArrayBuf(ConstantDef) global_constants = array_buf_new(ConstantDef)();
    while (parser->pos != tokens.tokens.len) {
        ConstantDef constant;
        if (parse_constant(parser, &constant) != SUCCESS) {
            parser_skip_until(parser, TOKEN_SEMICOLON);
            parser->pos++;
            continue;
        }
        array_buf_push(ConstantDef)(&global_constants, constant);
    }
    *dst = (Module){
        .global_constants = global_constants
    };
    return SUCCESS;
}

static Result parse_constant(Parser* parser, ConstantDef* dst) {
    Identifier name;
    if (parse_identifier(parser, &name) != SUCCESS) {
        return ERROR;
    }
    if (!parser_match(parser, TOKEN_COLON_COLON, NULL)) {
        return ERROR;
    }
    Expression value;
    if (parse_expression(parser, &value) != SUCCESS) {
        return ERROR;
    }
    if (!parser_match(parser, TOKEN_SEMICOLON, NULL)) {
        return ERROR;
    }
    *dst = (ConstantDef){
        .name = name,
        .explicitly_typed = false,
        .value = box_expression(parser, value),
    };
    return SUCCESS;
}

static Result parse_expression(Parser* parser, Expression* dst) {
    if (parser->tokens.tokens.len == parser->pos) {
        // FIXME: report error
        return ERROR;
    }
    Token head = parser->tokens.tokens.data[parser->pos];
    Result result = SUCCESS;
    switch (head.kind) {
    case TOKEN_FN:
        dst->kind = EXPRESSION_FUNCTION;
        if (
            parse_function(parser, &dst->as.function, &dst->range)
            != SUCCESS
        ) {
            return ERROR;
        }
        break;

    case TOKEN_FALSE:
        dst->kind = EXPRESSION_LITERAL_BOOL;
        dst->as.literal_bool = false;
        dst->range = token_range(parser->tokens, head);
        parser->pos++;
        break;
    case TOKEN_TRUE:
        dst->kind = EXPRESSION_LITERAL_BOOL;
        dst->as.literal_bool = true;
        dst->range = token_range(parser->tokens, head);
        parser->pos++;
        break;

    default:
        return ERROR;
    }
    return SUCCESS;
}

static Result parse_function(Parser* parser, Function* dst, Range* range) {
    usize start = parser->pos++;
    Pattern input;
    if (parse_pattern(parser, &input) != SUCCESS) {
        return ERROR;
    }
    if (!parser_match(parser, TOKEN_ARROW, NULL)) {
        return ERROR;
    }
    TypeName output_type;
    if (parse_type_name(parser, &output_type) != SUCCESS) {
        return ERROR;
    }
    if (!parser_match(parser, TOKEN_DOUBLE_ARROW, NULL)) {
        return ERROR;
    }
    Expression output;
    if (parse_expression(parser, &output) != SUCCESS) {
        return ERROR;
    }
    usize end = parser->pos;
    *dst = (Function){
        .input = input,
        .explicit_output_type = true,
        .output_type_name = output_type,
        .output = box_expression(parser, output),
    };
    *range = token_range_range(parser->tokens, (Range){ start, end });
    return SUCCESS;
}

static Result parse_pattern(Parser* parser, Pattern* dst) {
    usize start = parser->pos;
    Identifier identifier;
    if (parse_identifier(parser, &identifier) != SUCCESS) {
        return ERROR;
    }
    if (!parser_match(parser, TOKEN_COLON, NULL)) {
        return ERROR;
    }
    TypeName type_name;
    if (parse_type_name(parser, &type_name) != SUCCESS) {
        return ERROR;
    }
    usize end = parser->pos;
    *dst = (Pattern){
        .kind = PATTERN_VARIABLE,
        .as.variable = {
            .name = identifier,
            .explicitly_typed = true,
            .type_name = type_name,
        },
        .explicitly_typed = true,
        .type_name = type_name,
        .range = token_range_range(parser->tokens, (Range){ start, end }),
    };
    return SUCCESS;
}

static Result parse_type_name(Parser* parser, TypeName* dst) {
    Identifier identifier;
    if (parse_identifier(parser, &identifier) != SUCCESS) {
        return ERROR;
    }
    *dst = (TypeName){
        .kind = TYPE_NAME_IDENTIFIER,
        .as.identifier = identifier,
        .range = identifier.range,
    };
    return SUCCESS;
}

static Result parse_identifier(Parser* parser, Identifier* dst) {
    Token identifier;
    if (!parser_match(parser, TOKEN_IDENTIFIER, &identifier)) {
        return ERROR;
    }
    *dst = (Identifier){
        .range = token_range(parser->tokens, identifier),
    };
    return SUCCESS;
}
