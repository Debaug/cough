#pragma once

#include <stdlib.h>
#include <limits.h>

#include "alloc/array.h"

typedef struct TextPos {
    usize line;    // 0-indexed
    usize column;  // 0-indexed
    usize index;
} TextPos;

TextPos text_pos_next(TextPos pos, const char* text);

typedef struct TextView {
    const char* data;
    usize len;
    TextPos start;
    TextPos end;
} TextView;

bool text_eq(TextView a, TextView b);
TextView text_view_disjoint_union(TextView a, TextView b);

typedef struct Source {
    char path[PATH_MAX];
    StringBuf text;
    UsizeArrayBuf line_start_indices;
} Source;

typedef int Errno; // FIXME: duplicate declaration
Errno load_source_file(const char* path, Source* dst);
void destroy_source(Source source);
