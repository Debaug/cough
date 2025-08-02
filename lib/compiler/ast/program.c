#include <string.h>

#include "compiler/diagnostics.h"
#include "compiler/ast/program.h"
#include "compiler/ast/debug.h"

Result parse_item_declaration(Parser* parser, ItemDeclaration* dst) {
    ParserAllocState state = parser_snapshot_alloc(*parser);

    Token name;
    if (!match_parser(parser, TOKEN_IDENTIFIER, &name)) {
        Token token = peek_parser(*parser);
        report_simple_compiler_error(
            parser->reporter,
            CE_INVALID_ITEM_DECLARATION,
            format(
                "invalid token in item declaration: expected an identifier, found `%.*s`",
                STRING_FMT(token.text)
            ),
            token.text
        );
        parser_restore_alloc(parser, state);
        return ERROR;
    }
    dst->name = name.text;

    if (!match_parser(parser, TOKEN_COLON_COLON, NULL)) {
        Token token = peek_parser(*parser);
        report_simple_compiler_error(
            parser->reporter,
            CE_INVALID_ITEM_DECLARATION,
            format(
                "invalid token in item declaration: expected `::`, found `%.*s`",
                STRING_FMT(token.text)
            ),
            token.text
        );
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    Result result;
    switch (peek_parser(*parser).kind) {
    case TOKEN_FN:
        dst->kind = ITEM_FUNCTION;
        result = parse_function(parser, &dst->as.function);
        break;
    case TOKEN_STRUCT:
        dst->kind = ITEM_STRUCT;
        result = parse_struct(parser, &dst->as.composite);
        break;
    case TOKEN_VARIANT:
        dst->kind = ITEM_VARIANT;
        result = parse_variant(parser, &dst->as.composite);
        break;
    default: result = ERROR;
    }

    if (result != SUCCESS) {
        parser_restore_alloc(parser, state);
        return ERROR;
    }

    return SUCCESS;
}

Result parse_program(Parser* parser, Program* program) {
    ItemDeclarationArrayBuf items = new_array_buf(ItemDeclaration);
    while (!parser_is_eof(*parser)) {
        ItemDeclaration item_declaration;
        if (parse_item_declaration(parser, &item_declaration) != SUCCESS) {
            // error gets reported in `parse_item_declaration`
            skip_parser_until(parser, TOKEN_IDENTIFIER);
            continue;
        }
        array_buf_push(&items, item_declaration);
    }
    ast_push_alloc(&parser->storage, items.data);
    *program = (Program){ .items = items };
    return SUCCESS;
}

typedef struct FunctionItem {
    TextView name;
    Function* function;
} FunctionItem;
typedef ArrayBuf(FunctionItem) FunctionItemArrayBuf;

void report_symbol_defined_multiple_times(Reporter* reporter, TextView name) {
    report_simple_compiler_error(
        reporter,
        CE_DUPLICATE_SYMBOL_NAME,
        format(
            "symbol with name `%.*s` was defined multiple times",
            STRING_FMT(name)
        ),
        name
    );
}

#if 0

Result analyze_unordered_symbols(
    Analyzer* analyzer,
    Program* program
) {
    // record custom type symbols
    for (usize i = 0; i < program->items.len; i++) {
        ItemDeclaration* item = &program->items.data[i];
        ElementTypeKind type_kind;
        switch (item->kind) {
        case ITEM_FUNCTION: continue;
        case ITEM_STRUCT: type_kind = TYPE_STRUCT; break;
        case ITEM_VARIANT: type_kind = TYPE_VARIANT; break;
        }
        Symbol symbol = {
            .name = item->name,
            .kind = SYMBOL_TYPE,
            .as.type = (ElementType){
                .kind = type_kind,
                .as.composite = &item->as.composite
            }
        };
        if (!add_symbol(analyzer->current_scope, symbol)) {
            report_symbol_defined_multiple_times(analyzer->reporter, item->name);
            return ERROR;
        }
    }

    for (usize i = 0; i < program->items.len; i++) {
        ItemDeclaration* item = &program->items.data[i];
        switch (item->kind) {
        case ITEM_FUNCTION:;
            Symbol symbol = {
                .name = item->name,
                .kind = SYMBOL_FUNCTION,
                .as.function = &item->as.function
            };
            if (!add_symbol(analyzer->current_scope, symbol)) {
                report_symbol_defined_multiple_times(analyzer->reporter, item->name);
                return ERROR;
            }
            FunctionSignature* signature = &item->as.function.signature;
            if (signature->has_return_type) {
                exit(-1); // TODO
            }
            break;

        case ITEM_STRUCT:
        case ITEM_VARIANT:
            analyze_composite(analyzer, &item->as.composite);
            break;
        }
    }

    return SUCCESS;
}

Result analyze_expressions(Analyzer* analyzer, Program* program) {
    for (usize i = 0; i < program->items.len; i++) {
        ItemDeclaration* declaration = &program->items.data[i];
        if (declaration->kind != ITEM_FUNCTION) {
            continue;
        }
        analyze_function(analyzer, &declaration->as.function);
    }
    return SUCCESS;
}

#endif

void debug_item_declaration(
    ItemDeclaration item_declaration,
    AstDebugger* debugger
) {
    ast_debug_start(debugger, "item_declaration");

    ast_debug_key(debugger, "name");
    ast_debug_string_view(debugger, STRING_VIEW(item_declaration.name));

#define DEBUG_ITEM_DECLARATION_CASE(kind, dbg, name, str)   \
    case kind:                                              \
        ast_debug_key(debugger, str);                       \
        dbg (item_declaration.as.name, debugger);           \
        break;

    switch (item_declaration.kind) {
    DEBUG_ITEM_DECLARATION_CASE(ITEM_FUNCTION, debug_function, function, "function")
    DEBUG_ITEM_DECLARATION_CASE(ITEM_STRUCT, debug_struct, composite, "struct")
    DEBUG_ITEM_DECLARATION_CASE(ITEM_VARIANT, debug_variant, composite, "variant")
    }

    ast_debug_end(debugger);
}

void debug_program(
    Program program,
    AstDebugger* debugger
) {
    ast_debug_start(debugger, "program");
    ast_debug_key(debugger, "items");
    ast_debug_start_sequence(debugger);
    for (usize i = 0; i < program.items.len; i++) {
        debug_item_declaration(
            program.items.data[i],
            debugger
        );
    }
    ast_debug_end_sequence(debugger);
    ast_debug_end(debugger);
}
