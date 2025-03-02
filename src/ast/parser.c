#include "ast/parser.h"

parser_t new_parser(const token_t* tokens, reporter_t* reporter) {
    return (parser_t){
        .tokens = tokens,
        .pos = 0,
        .storage = new_ast_storage(),
        .reporter = reporter
    };
}

token_t peek_parser(parser_t parser) {
    return peek_parser_nth(parser, 0);
}

token_t peek_parser_nth(parser_t parser, size_t offset) {
    return parser.tokens[parser.pos + offset];
}

void step_parser(parser_t* parser) {
    parser->pos++;
}

void step_parser_by(parser_t* parser, size_t steps) {
    parser->pos += steps;
}

parser_alloc_state_t parser_snapshot(parser_t parser) {
    return (parser_alloc_state_t){
        .arena_stack_state = arena_stack_snapshot(parser.storage.arena_stack),
        .allocations_state = alloc_stack_snapshot(parser.storage.alloc_stack)
    };
}

void parser_restore(parser_t* parser, parser_alloc_state_t state) {
    arena_stack_restore(&parser->storage.arena_stack, state.arena_stack_state);
    alloc_stack_restore(&parser->storage.alloc_stack, state.allocations_state);
}

bool match_parser(parser_t* parser, token_type_t pattern, token_t* dst) {
    return match_parser_sequence(parser, &pattern, dst, 1) == 1;
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

text_view_t skip_parser_until(parser_t* parser, token_type_t token) {
    return skip_parser_until_any_of(parser, &token, 1);
}

static void match_closing_to_opening(token_type_array_buf_t* group_delimiters, token_type_t matching_left_token) {
    if (group_delimiters->len == 0) {
        return;
    }
    if (group_delimiters->data[group_delimiters->len - 1] == (matching_left_token)) {
        array_buf_pop(group_delimiters, NULL, token_t);
    }
}

text_view_t skip_parser_until_any_of(parser_t* parser, token_type_t* tokens, size_t ntokens) {
    text_view_t start = peek_parser(*parser).text;

    token_type_array_buf_t group_delimiters = new_array_buf();
    while (!parser_is_eof(*parser)) {
        token_t token = peek_parser(*(parser));
        if (group_delimiters.len == 0) {
            bool found_match = false;
            for (size_t i = 0; i < ntokens; i++) {
                if (token.type == tokens[i]) {
                    found_match = true;
                    break;
                }
            }
            if (found_match) {
                break;
            }
        }

        switch (token.type) {
        case TOKEN_LEFT_PAREN:
        case TOKEN_LEFT_BRACKET:
        case TOKEN_LEFT_BRACE:
            array_buf_push(&group_delimiters, token.type);
            break;
        
        case TOKEN_RIGHT_PAREN:
            match_closing_to_opening(&group_delimiters, TOKEN_LEFT_PAREN);
            break;
        case TOKEN_RIGHT_BRACKET:
            match_closing_to_opening(&group_delimiters, TOKEN_LEFT_BRACKET);
            break;
        case TOKEN_RIGHT_BRACE:
            match_closing_to_opening(&group_delimiters, TOKEN_LEFT_BRACKET);
            break;

        default: break;
        }

        step_parser(parser);
    }
    free_array_buf(group_delimiters);

    text_view_t end = peek_parser(*parser).text;
    return text_view_disjoint_union(start, end);
}
