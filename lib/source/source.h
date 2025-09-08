#pragma once

#include <stdio.h>
#include <limits.h>

#include "primitives/primitives.h"
#include "collections/array.h"
#include "diagnostics/errno.h"

typedef struct LineColumn {
    usize line;
    usize column;
} LineColumn;

typedef struct SourceText {
    char const* path;   // NULL if stdin
    char const* text;
    ArrayBuf(usize) line_indices;
} SourceText;

Errno read_file(FILE* file, ArrayBuf(char)* dst);

SourceText source_text_new(char const* path, char const* text);
void source_text_free(SourceText* source);

LineColumn source_text_position(SourceText text, usize index);
