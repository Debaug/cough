#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "text/text.h"
#include "diagnostics/diagnostics.h"

TextPos text_pos_next(TextPos pos, const char* text) {
    if (text[0] == '\n') {
        return (TextPos){ 
            .line = pos.line + 1,
            .column = 0,
            .index = pos.index + 1,
        };
    } else {
        return (TextPos){
            .line = pos.line,
            .column = pos.column + 1,
            .index = pos.index + 1,
        };
    }
}

bool text_eq(TextView a, TextView b) {
    if (a.len != b.len) {
        return false;
    }
    return strncmp(a.data, b.data, a.len) == 0;
}

TextView text_view_disjoint_union(TextView a, TextView b) {
    if (a.data >= b.data) {
        TextView tmp = a;
        a = b;
        b = tmp;
    }
    return (TextView){
        .data = a.data,
        .len = b.end.index - a.start.index,
        .start = a.start,
        .end = b.end,
    };
}

Errno load_source_file(const char* path, Source* dst) {
    FILE* input;
    char absolute_path[PATH_MAX];
    if (path == NULL) {
        input = stdin;
        strcpy(absolute_path, "<stdin>");
    } else {
        if (realpath(path, absolute_path) == NULL) {
            return errno;
        }
        input = fopen(path, "r");
        if (input == NULL) {
            return errno;
        }
    }

    StringBuf text = new_array_buf(char);
    if (read_file(input, &text) != SUCCESS) {
        return errno;
    }
    if (text.data[text.len - 1] != '\n') {
        char newline_char = '\n';
        array_buf_push(&text, newline_char);
    }

    strcpy(dst->path, absolute_path);
    dst->text = text;

    if (path != NULL) {
        if (fclose(input) != SUCCESS) {
            return errno;
        }
    }

    dst->line_start_indices = new_array_buf(usize);
    usize start = 0;
    array_buf_push(&dst->line_start_indices, start);
    for (usize i = 0; i < text.len; i++) {
        if (text.data[i] == '\n') {
            usize next_idx = i + 1;
            array_buf_push(&dst->line_start_indices, next_idx);
        }
    }

    return 0;
}
