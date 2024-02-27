#pragma once

#include <stdbool.h>

#include "tokens/token.h"

typedef struct scanner {
    const char* text;
    text_pos_t text_pos;
} scanner_t;

typedef enum scan_result {
    SCAN_SUCCESS,
    SCAN_UNEXPECTED_CHARACTER,
    SCAN_INCOMPLETE_TOKEN,
} scan_result_t;

scanner_t new_scanner(const char* text);
char peek_scanner(scanner_t scanner);
void step_scanner(scanner_t* scanner);
scan_result_t scan_punctuation(scanner_t* scanner, token_t* dst);
token_array_buf_t scan(scanner_t* scanner);
