#pragma once

#include <stdlib.h>

#include "text.h"

typedef struct text_pos {
    size_t line;    // 0-indexed
    size_t column;  // 0-indexed
    size_t index;   // 0-indexed, position of character
} text_pos_t;

text_pos_t text_pos_next(text_pos_t pos, const char* text);

typedef struct text_span {
    text_pos_t start;
    text_pos_t end;
} text_span_t;
