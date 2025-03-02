#pragma once

#include <stdbool.h>

#include "tokens/token.h"
#include "diagnostic/diagnostic.h"

typedef struct scanner {
    const char* text;
    text_pos_t text_pos;
    reporter_t* reporter;
} scanner_t;

scanner_t new_scanner(const char* text, reporter_t* reporter);
char peek_scanner(scanner_t scanner);
void step_scanner(scanner_t* scanner);
text_view_t scanner_text_from(scanner_t scanner, text_pos_t start);
result_t scan_punctuation(scanner_t* scanner, token_t* dst);
token_array_buf_t scan(scanner_t* scanner);
