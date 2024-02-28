#include "ast/analyzer.h"

static scope_t new_scope(scope_t* parent) {
    return (scope_t){
        .symbols = new_array_buf(),
        .children = new_array_buf(),
        .parent = parent,
    };
}

analyzer_t new_analyzer(void) {
    scope_t* root = malloc(sizeof(scope_t));
    *root = new_scope(NULL);
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
