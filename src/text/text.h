#pragma once

#include <stdlib.h>
#include <limits.h>

#include "alloc/array.h"

typedef struct text_pos {
    size_t line;    // 0-indexed
    size_t column;  // 0-indexed
    size_t index;
} text_pos_t;

text_pos_t text_pos_next(text_pos_t pos, const char* text);

typedef struct text_view {
    const char* data;
    size_t len;
    text_pos_t start;
    text_pos_t end;
} text_view_t;

bool text_eq(text_view_t a, text_view_t b);
text_view_t text_view_disjoint_union(text_view_t a, text_view_t b);
#define TEXT_FMT(text_view) (int)(text_view).len, (text_view).data

typedef struct source {
    char path[PATH_MAX];
    string_buf_t text;
    size_array_buf_t line_start_indices;
} source_t;

typedef int errno_t; // FIXME: duplicate declaration
errno_t load_source_file(const char* path, source_t* dst);
errno_t destroy_source(source_t source);
