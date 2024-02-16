#pragma once

#include <stdlib.h>

#include "util/array.h"

typedef struct text_pos {
    size_t line;    // 0-indexed
    size_t column;  // 0-indexed
} text_pos_t;

text_pos_t text_pos_next(text_pos_t pos, const char* text);

typedef struct text_view {
    const char* ptr;
    size_t len;
    text_pos_t start;
    text_pos_t end;
} text_view_t;
