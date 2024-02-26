#pragma once

#include <stdbool.h>

#include "scanner/scanner.h"

#include "util/arena_stack.h"
#include "util/alloc_stack.h"

typedef struct ast_storage {
    arena_stack_t arena_stack;
    alloc_stack_t allocations;
} ast_storage_t;

#define ast_box(storage, val) arena_stack_push(&(storage)->arena_stack, val)

typedef enum parse_result {
    PARSE_SUCCESS,
    PARSE_ERROR,
} parse_result_t;

typedef struct parser {
    const token_t* tokens;
    size_t pos;
    ast_storage_t storage;
} parser_t;

parser_t new_parser(const token_t* tokens);

token_t peek_parser(parser_t parser);
void step_parser(parser_t* parser);
void step_parser_by(parser_t* parser, size_t steps);

typedef struct parser_state {
    size_t pos;
    arena_stack_state_t arena_stack_state;
    size_t allocations_state;
} parser_state_t;

parser_state_t parser_snapshot(parser_t parser);
void parser_restore(parser_t* parser, parser_state_t state);

#define PARSER_ERROR_RESTORE(parser, state) do {    \
    parser_restore(parser, state);                  \
    return PARSE_ERROR;                             \
} while(0)

bool match_parser(
    parser_t* parser,
    token_type_t type,
    token_t* dst
);
size_t match_parser_sequence(
    parser_t* parser,
    const token_type_t* pattern,
    token_t* dst,
    size_t len
);

bool parser_is_eof(parser_t parser);
