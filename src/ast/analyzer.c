#include <string.h>

#include "ast/analyzer.h"

static scope_t new_scope(scope_t* parent) {
    return (scope_t){
        .symbols = new_array_buf(),
        .children = new_array_buf(),
        .parent = parent,
    };
}

const symbol_t* find_symbol(scope_t leaf_scope, string_view_t name) {
    scope_t* scope = &leaf_scope;
    do {
        for (size_t i = 0; i < scope->symbols.len; i++) {
            if (string_views_eq(STRING_VIEW(scope->symbols.data[i].name), name)) {
                return &scope->symbols.data[i];
            }
        }
        scope = scope->parent;
    } while(scope != NULL);
    return NULL;
}

const symbol_t* find_symbol_of(scope_t scope, string_view_t name, symbol_kind_t mask) {
    const symbol_t* symbol = find_symbol(scope, name);
    if (symbol == NULL) {
        return NULL;
    }
    if ((symbol->kind & mask) == 0) {
        return NULL;
    }
    return symbol;
}

bool add_symbol(scope_t* scope, symbol_t symbol) {
    for (size_t i = 0; i < scope->symbols.len; i++) {
        if (text_eq(symbol.name, scope->symbols.data[i].name)) {
            return false;
        }
    }
    array_buf_push(&scope->symbols, symbol);
    return true;
}

static symbol_t default_symbols[] = {
    {
        .name = (text_view_t){ .data = "Never", .len = 5 },
        .kind = SYMBOL_TYPE,
        .as.type = (element_type_t){ .kind = TYPE_NEVER }
    },
    {
        .name = (text_view_t){ .data = "Unit", .len = 4 },
        .kind = SYMBOL_TYPE,
        .as.type = (element_type_t){ .kind = TYPE_UNIT }
    },
    {
        .name = (text_view_t){ .data = "Bool", .len = 4 },
        .kind = SYMBOL_TYPE,
        .as.type = (element_type_t){ .kind = TYPE_BOOL }
    },
    {
        .name = (text_view_t){ .data = "Int", .len = 3 },
        .kind = SYMBOL_TYPE,
        .as.type = (element_type_t){ .kind = TYPE_INT }
    },
    {
        .name = (text_view_t){ .data = "Float", .len = 5 },
        .kind = SYMBOL_TYPE,
        .as.type = (element_type_t){ .kind = TYPE_FLOAT }
    }
};

analyzer_t new_analyzer(void) {
    scope_t* root = malloc(sizeof(scope_t));
    *root = new_scope(NULL);
    array_buf_extend(&root->symbols, default_symbols, 5, symbol_t);
    return (analyzer_t){ .root = root, .current_scope = root };
}

static void free_scope(scope_t scope) {
    free_array_buf(scope.symbols);
    for (size_t i = 0; i < scope.children.len; i++) {
        free_scope(scope.children.data[i]);
    }
    free_array_buf(scope.children);
}

void free_analyzer(analyzer_t analyzer) {
    free_scope(*analyzer.root);
    free(analyzer.root);
}

scope_t* analyzer_enter_new_scope(analyzer_t* analyzer) {
    scope_t child = new_scope(analyzer->current_scope);
    scope_array_buf_t* children = &analyzer->current_scope->children;
    array_buf_push(children, child);
    analyzer->current_scope = &children->data[children->len - 1];
    return analyzer->current_scope;
}

void analyzer_exit_scope(analyzer_t* analyzer) {
    analyzer->current_scope = analyzer->current_scope->parent;
}

analyze_result_t analyze_variable(analyzer_t* analyzer, variable_t* variable) {
    eprintf("TODO: analyze_variable");
    abort();

    // analyze_field(analyzer, (field_t*)variable);
    // symbol_t symbol = (symbol_t){
    //     .name = variable->name,
    //     .kind = SYMBOL_VARIABLE,
    //     .as.variable = variable,
    // };
    // if (!add_symbol(analyzer->current_scope, symbol)) {
    //     // where error?
    //     report_error("symbol with name `%.*s` already exists", TEXT_FMT(symbol.name));
    //     return ANALYZE_ERROR;
    // }
    return ANALYZE_SUCCESS;
}

analyze_result_t analyze_field(analyzer_t* analyzer, field_t* field) {
    eprintf("TODO: analyze_field");
    abort();
    
    // const symbol_t* type_symbol = find_symbol_of(
    //     *analyzer->current_scope,
    //     STRING_VIEW(field->type.element_type_name),
    //     SYMBOL_TYPE
    // );
    // if (type_symbol == NULL) {
    //     // where error?
    //     report_error("type `%.*s` not found",
    //         TEXT_FMT(field->type.element_type_name));
    // } else {
    //     field->type.type.element_type = type_symbol->as.type;
    // }
    return ANALYZE_SUCCESS;
}

analyze_result_t analyze_composite(
    analyzer_t* analyzer,
    composite_type_t* composite
) {
    eprintf("TODO: analyze_composite\n");
    exit(EXIT_FAILURE);
    return ANALYZE_SUCCESS;

    for (size_t i = 0; i < composite->fields.len; i++) {
        field_t* field = &composite->fields.data[i];
        analyze_field(analyzer, field);
        for (size_t j = 0; j < i; j++) {
            if (text_eq(field->name, composite->fields.data[j].name)) {
                // where error?
                // report_error(
                //     "field `%.*s` defined multiple times",
                //     TEXT_FMT(field->name)
                // );
                break;
            }
        }
    }
    return ANALYZE_SUCCESS;
}
