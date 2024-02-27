#include "ast/parser.h"

parser_t new_parser(const token_t* tokens) {
    return (parser_t){ .tokens = tokens, .pos = 0, .storage = new_ast_storage() };
}

token_t peek_parser(parser_t parser) {
    return parser.tokens[parser.pos];
}

void step_parser(parser_t* parser) {
    parser->pos++;
}

void step_parser_by(parser_t* parser, size_t steps) {
    parser->pos += steps;
}

parser_state_t parser_snapshot(parser_t parser) {
    return (parser_state_t){
        .pos = parser.pos,
        .arena_stack_state = arena_stack_snapshot(parser.storage.arena_stack),
        .allocations_state = alloc_stack_snapshot(parser.storage.alloc_stack)
    };
}

void parser_restore(parser_t* parser, parser_state_t state) {
    parser->pos = state.pos;
    arena_stack_restore(&parser->storage.arena_stack, state.arena_stack_state);
    alloc_stack_restore(&parser->storage.alloc_stack, state.allocations_state);
}

bool match_parser(parser_t* parser, token_type_t pattern, token_t* dst) {
    return (bool)match_parser_sequence(parser, &pattern, dst, 1);
}

size_t match_parser_sequence(parser_t* parser, const token_type_t* pattern, token_t* dst, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        token_t token = parser->tokens[parser->pos + i];
        if (token.type != pattern[i] || token.type == TOKEN_EOF) {
            break;
        }
        if (dst) {
            dst[i] = token;
        }
    }
    if (i == len) {
        parser->pos += i;
    }
    return i;
}

bool parser_is_eof(parser_t parser) {
    return peek_parser(parser).type == TOKEN_EOF;
}
