#pragma once

#include "alloc/array.h"
#include "compiler/ast/function.h"
#include "compiler/ast/parser.h"
#include "compiler/ast/debug.h"

typedef enum ItemDeclarationKind {
    ITEM_FUNCTION,
    ITEM_STRUCT,
    ITEM_VARIANT,
} ItemDeclarationKind;

typedef struct ItemDeclaration {
    TextView name;
    ItemDeclarationKind kind;
    union {
        Function function;
        CompositeType composite;
    } as;
} ItemDeclaration;

typedef ArrayBuf(ItemDeclaration) ItemDeclarationArrayBuf;
typedef struct Program {
    ItemDeclarationArrayBuf items;
} Program;

Result parse_item_declaration(Parser* parser, ItemDeclaration* dst);
Result parse_program(Parser* parser, Program* program);

Result analyze_unordered_symbols(Analyzer* analyzer, Program* program);
Result analyze_expressions(Analyzer* analyzer, Program* program);

void debug_program(Program program, AstDebugger* debugger);
void debug_item_declaration(
    ItemDeclaration item_declaration,
    AstDebugger* debugger
);
