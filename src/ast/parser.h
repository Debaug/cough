#pragma once

#include <stdbool.h>

#include "tokens/scanner.h"
#include "ast/storage.h"
#include "diagnostic/diagnostic.h"

typedef struct parser {
    const token_t* tokens;
    size_t pos;
    ast_storage_t storage;
    reporter_t* reporter;
} parser_t;

parser_t new_parser(const token_t* tokens, reporter_t* reporter);

token_t peek_parser(parser_t parser);
token_t peek_parser_nth(parser_t parser, size_t offset);
void step_parser(parser_t* parser);
void step_parser_by(parser_t* parser, size_t steps);

bool parser_is_eof(parser_t parser);

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

text_view_t skip_parser_until(parser_t* parser, token_type_t token);
text_view_t skip_parser_until_any_of(parser_t* parser, token_type_t* tokens, size_t len);

#define parser_error_restore(parser, state, error) do { \
    parser_restore((parser), (state));                  \
    report((parser)->reporter, (error));                \
} while (0)

typedef struct parser_alloc_state {
    arena_stack_state_t arena_stack_state;
    size_t allocations_state;
} parser_alloc_state_t;

parser_alloc_state_t parser_snapshot(parser_t parser);
void parser_restore(parser_t* parser, parser_alloc_state_t state);
