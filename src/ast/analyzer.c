#include <string.h>

#include "ast/analyzer.h"

static Scope new_scope(Scope* parent) {
    return (Scope){
        .symbols = new_array_buf(),
        .children = new_array_buf(),
        .parent = parent,
    };
}

const Symbol* find_symbol(Scope leaf_scope, StringView name) {
    Scope* scope = &leaf_scope;
    do {
        for (usize i = 0; i < scope->symbols.len; i++) {
            if (string_views_eq(STRING_VIEW(scope->symbols.data[i].name), name)) {
                return &scope->symbols.data[i];
            }
        }
        scope = scope->parent;
    } while(scope != NULL);
    return NULL;
}

const Symbol* find_symbol_of(Scope scope, StringView name, SymbolKind mask) {
    const Symbol* symbol = find_symbol(scope, name);
    if (symbol == NULL) {
        return NULL;
    }
    if ((symbol->kind & mask) == 0) {
        return NULL;
    }
    return symbol;
}

bool add_symbol(Scope* scope, Symbol symbol) {
    for (usize i = 0; i < scope->symbols.len; i++) {
        if (text_eq(symbol.name, scope->symbols.data[i].name)) {
            return false;
        }
    }
    array_buf_push(&scope->symbols, symbol);
    return true;
}

static Symbol default_symbols[] = {
    {
        .name = (TextView){ .data = "Never", .len = 5 },
        .kind = SYMBOL_TYPE,
        .as.type = (ElementType){ .kind = TYPE_NEVER }
    },
    {
        .name = (TextView){ .data = "Unit", .len = 4 },
        .kind = SYMBOL_TYPE,
        .as.type = (ElementType){ .kind = TYPE_UNIT }
    },
    {
        .name = (TextView){ .data = "Bool", .len = 4 },
        .kind = SYMBOL_TYPE,
        .as.type = (ElementType){ .kind = TYPE_BOOL }
    },
    {
        .name = (TextView){ .data = "Int", .len = 3 },
        .kind = SYMBOL_TYPE,
        .as.type = (ElementType){ .kind = TYPE_INT }
    },
    {
        .name = (TextView){ .data = "Float", .len = 5 },
        .kind = SYMBOL_TYPE,
        .as.type = (ElementType){ .kind = TYPE_FLOAT }
    }
};

Analyzer new_analyzer(Reporter* reporter) {
    Scope* root = malloc(sizeof(Scope));
    *root = new_scope(NULL);
    array_buf_extend(&root->symbols, default_symbols, 5, Symbol);
    return (Analyzer){
        .root = root,
        .current_scope = root,
        .reporter = reporter
    };
}

static void free_scope(Scope scope) {
    free_array_buf(scope.symbols);
    for (usize i = 0; i < scope.children.len; i++) {
        free_scope(scope.children.data[i]);
    }
    free_array_buf(scope.children);
}

void free_analyzer(Analyzer analyzer) {
    free_scope(*analyzer.root);
    free(analyzer.root);
}

Scope* analyzer_enter_new_scope(Analyzer* analyzer) {
    Scope child = new_scope(analyzer->current_scope);
    ScopeArrayBuf* children = &analyzer->current_scope->children;
    array_buf_push(children, child);
    analyzer->current_scope = &children->data[children->len - 1];
    return analyzer->current_scope;
}

void analyzer_exit_scope(Analyzer* analyzer) {
    analyzer->current_scope = analyzer->current_scope->parent;
}

// analyze_result_t analyze_variable(Analyzer* analyzer, Variable* variable) {
//     eprintf("TODO: analyze_variable");
//     abort();

//     // analyze_field(analyzer, (Field*)variable);
//     // Symbol symbol = (Symbol){
//     //     .name = variable->name,
//     //     .kind = SYMBOL_VARIABLE,
//     //     .as.variable = variable,
//     // };
//     // if (!add_symbol(analyzer->current_scope, symbol)) {
//     //     // where error?
//     //     report_error("symbol with name `%.*s` already exists", TEXT_FMT(symbol.name));
//     //     return ANALYZE_ERROR;
//     // }
//     return ANALYZE_SUCCESS;
// }

// analyze_result_t analyze_field(Analyzer* analyzer, Field* field) {
//     eprintf("TODO: analyze_field");
//     abort();
    
//     // const Symbol* type_symbol = find_symbol_of(
//     //     *analyzer->current_scope,
//     //     STRING_VIEW(field->type.element_type_name),
//     //     SYMBOL_TYPE
//     // );
//     // if (type_symbol == NULL) {
//     //     // where error?
//     //     report_error("type `%.*s` not found",
//     //         TEXT_FMT(field->type.element_type_name));
//     // } else {
//     //     field->type.type.element_type = type_symbol->as.type;
//     // }
//     return ANALYZE_SUCCESS;
// }

// analyze_result_t analyze_composite(
//     Analyzer* analyzer,
//     CompositeType* composite
// ) {
//     eprintf("TODO: analyze_composite\n");
//     exit(EXIT_FAILURE);
//     return ANALYZE_SUCCESS;

//     for (usize i = 0; i < composite->fields.len; i++) {
//         Field* field = &composite->fields.data[i];
//         analyze_field(analyzer, field);
//         for (usize j = 0; j < i; j++) {
//             if (text_eq(field->name, composite->fields.data[j].name)) {
//                 // where error?
//                 // report_error(
//                 //     "field `%.*s` defined multiple times",
//                 //     TEXT_FMT(field->name)
//                 // );
//                 break;
//             }
//         }
//     }
//     return ANALYZE_SUCCESS;
// }
