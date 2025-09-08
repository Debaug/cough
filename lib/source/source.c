#include "source/source.h"

Errno read_file(FILE* file, ArrayBuf(char)* dst) {
    usize len = 0;
    while (!feof(file)) {
        array_buf_reserve(char)(dst, (len == 0) ? 64 : len);
        usize additional =
            fread(dst->data + dst->len, 1, dst->capacity - dst->len, file);
        if (additional == 0) {
            return errno;
        }
        dst->len += additional;
    }

    return 0;
}

SourceText source_text_new(char const* path, char const* text) {
    ArrayBuf(usize) line_indices;
    array_buf_push(usize)(&line_indices, 0);
    usize i;
    for (i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            array_buf_push(usize)(&line_indices, i + 1);
        }
    }
    array_buf_push(usize)(&line_indices, i);
    return (SourceText){
        .path = path,
        .text = text,
        .line_indices = line_indices
    };
}

void source_text_free(SourceText* source) {
    array_buf_free(char)(&source->line_indices);
}

LineColumn source_text_position(SourceText text, usize index) {
    // invariants during the loop:
    // index at `start` <= `index` < index at `end`

    usize start = 0;
    usize end = text.line_indices.len;
    usize line_index;
    while (true) {
        usize i = (start + end) / 2;
        if (index < text.line_indices.data[i]) {                // (1)
            end = i;
            // we still have: `index` < index at `end`.
        } else if (text.line_indices.data[i + 1] <= index) {    // (2)
            start = i + 1;
            // we still have: index at `start` <= index.
        } else {
            // we have: index at `i` <= `index` < index at `i + 1`.
            // hence the index at `i` is the index of the start of the searched
            // line.
            line_index = text.line_indices.data[i];
            break;
        }

        // Let s and e be respectively `start` and `end` before one iteration.
        // Let s' and e' be respectively `start` and `end` after the iteration.
        // If we didn't break, we have either:
        // (1) e' - s'  == floor((s + e) / 2) - s <= (e - s) / 2 < e - s
        // (2) e' - s'  == e - floor((s + e) / 2) - 1 <= (e - s) / 2 < e - s
        // Note: the last inequality of each case is strict because start < end
        // by the invariants.
        // Hence, by infinite descent, we are guaranteed to break.
    }

    return (LineColumn){ .line = line_index, .column = index - line_index };
}
