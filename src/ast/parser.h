#pragma once

#include <stdbool.h>

#include "scanner/scanner.h"

#include "ast/parse_result.h"

typedef struct parser {
    const token_t* tokens;
    size_t pos;
} parser_t;

parser_t new_parser(const token_t* tokens);

token_t peek_parser(parser_t parser);
void step_parser(parser_t* parser);
void step_parser_by(parser_t* parser, size_t steps);

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
