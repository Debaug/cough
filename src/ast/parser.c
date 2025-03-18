#include "ast/parser.h"

Parser new_parser(const Token* tokens, Reporter* reporter) {
    return (Parser){
        .tokens = tokens,
        .pos = 0,
        .storage = new_ast_storage(),
        .reporter = reporter
    };
}

Token peek_parser(Parser parser) {
    return peek_parser_nth(parser, 0);
}

Token peek_parser_nth(Parser parser, usize offset) {
    return parser.tokens[parser.pos + offset];
}

void step_parser(Parser* parser) {
    parser->pos++;
}

void step_parser_by(Parser* parser, usize steps) {
    parser->pos += steps;
}

ParserAllocState parser_snapshot_alloc(Parser parser) {
    return (ParserAllocState){
        .arena_stack_state = arena_stack_snapshot(parser.storage.arena_stack),
        .allocations_state = alloc_stack_snapshot(parser.storage.alloc_stack)
    };
}

void parser_restore_alloc(Parser* parser, ParserAllocState state) {
    arena_stack_restore(&parser->storage.arena_stack, state.arena_stack_state);
    alloc_stack_restore(&parser->storage.alloc_stack, state.allocations_state);
}

bool match_parser(Parser* parser, TokenKind pattern, Token* dst) {
    return match_parser_sequence(parser, &pattern, dst, 1) == 1;
}

usize match_parser_sequence(Parser* parser, const TokenKind* pattern, Token* dst, usize len) {
    usize i;
    for (i = 0; i < len; i++) {
        Token token = parser->tokens[parser->pos + i];
        if (token.kind != pattern[i] || token.kind == TOKEN_EOF) {
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

bool parser_is_eof(Parser parser) {
    return peek_parser(parser).kind == TOKEN_EOF;
}

TextView skip_parser_until(Parser* parser, TokenKind token) {
    return skip_parser_until_any_of(parser, &token, 1);
}

static void match_closing_to_opening(TokenKindArrayBuf* group_delimiters, TokenKind matching_left_token) {
    if (group_delimiters->len == 0) {
        return;
    }
    if (group_delimiters->data[group_delimiters->len - 1] == (matching_left_token)) {
        array_buf_pop(group_delimiters, NULL);
    }
}

TextView skip_parser_until_any_of(Parser* parser, TokenKind* tokens, usize ntokens) {
    TextView start = peek_parser(*parser).text;

    TokenKindArrayBuf group_delimiters = new_array_buf();
    while (!parser_is_eof(*parser)) {
        Token token = peek_parser(*(parser));
        if (group_delimiters.len == 0) {
            bool found_match = false;
            for (usize i = 0; i < ntokens; i++) {
                if (token.kind == tokens[i]) {
                    found_match = true;
                    break;
                }
            }
            if (found_match) {
                break;
            }
        }

        switch (token.kind) {
        case TOKEN_LEFT_PAREN:
        case TOKEN_LEFT_BRACKET:
        case TOKEN_LEFT_BRACE:
            array_buf_push(&group_delimiters, token.kind);
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

    TextView end = peek_parser(*parser).text;
    return text_view_disjoint_union(start, end);
}
