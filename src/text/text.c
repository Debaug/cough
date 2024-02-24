#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "text/text.h"
#include "diagnostic/diagnostic.h"

text_pos_t text_pos_next(text_pos_t pos, const char* text) {
    if (text[0] == '\n') {
        return (text_pos_t){ 
            .line = pos.line + 1,
            .column = 0,
        };
    } else {
        return (text_pos_t){
            .line = pos.line,
            .column = pos.column + 1,
        };
    }
}

errno_t load_source_file(const char* path, source_t* dst) {
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

    string_buf_t text = new_array_buf();
    if (read_file(input, &text) != SUCCESS) {
        return errno;
    }

    strcpy(dst->path, absolute_path);
    dst->text = text;

    if (path != NULL) {
        if (fclose(input) != SUCCESS) {
            return errno;
        }
    }
    return 0;
}
