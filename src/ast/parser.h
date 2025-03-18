#pragma once

#include <stdbool.h>

#include "tokens/scanner.h"
#include "ast/storage.h"
#include "diagnostic/diagnostic.h"

typedef struct Parser {
    const Token* tokens;
    usize pos;
    AstStorage storage;
    Reporter* reporter;
} Parser;

Parser new_parser(const Token* tokens, Reporter* reporter);

Token peek_parser(Parser parser);
Token peek_parser_nth(Parser parser, usize offset);
void step_parser(Parser* parser);
void step_parser_by(Parser* parser, usize steps);

bool parser_is_eof(Parser parser);

bool match_parser(
    Parser* parser,
    TokenKind type,
    Token* dst
);
usize match_parser_sequence(
    Parser* parser,
    const TokenKind* pattern,
    Token* dst,
    usize len
);

TextView skip_parser_until(Parser* parser, TokenKind token);
TextView skip_parser_until_any_of(Parser* parser, TokenKind* tokens, usize len);

#define parser_error_restore_alloc(parser, state, error) do {   \
    parser_restore_alloc((parser), (state));                    \
    report((parser)->reporter, (error));                        \
} while (0)

typedef struct ParserAllocState {
    ArenaStackState arena_stack_state;
    usize allocations_state;
} ParserAllocState;

ParserAllocState parser_snapshot_alloc(Parser parser);
void parser_restore_alloc(Parser* parser, ParserAllocState state);
