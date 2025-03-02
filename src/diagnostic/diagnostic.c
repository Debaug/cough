#include <string.h>
#include <errno.h>

#include "diagnostic/diagnostic.h"

void print_errno(void) {
    print_system_error("%s (errno %d)", strerror(errno), errno);
}

static void print_error_source(const source_t* source, text_view_t span) {
    bool snip;
    bool single_line;
    if (span.start.line == span.end.line) {
        snip = false;
        single_line = true;
    } else if (span.start.line - span.end.line < 3) {
        snip = false;
        single_line = false;
    } else {
        snip = true;
        single_line = false;
    }

    if (single_line) {
        eprintf(
            "at %s:%zu:%zu-%zu:\n",
            source->path,
            span.start.line + 1,
            span.start.column + 1,
            span.end.column + 1
        );
    } else {
        eprintf(
            "at %s:%zu:%zu-%zu:%zu:\n",
            source->path,
            span.start.line + 1,
            span.start.column + 1,
            span.end.line + 1,
            span.end.column + 1
        );
    }

    size_t* line_start_indices = source->line_start_indices.data;
    if (snip) {
        const char* first_line = source->text.data + line_start_indices[span.start.line];
        int head_len = span.start.column;
        int first_line_len = line_start_indices[span.start.line + 1] - line_start_indices[span.start.line];
        int first_line_error_len = first_line_len - head_len;

        const char* last_line = source->text.data + line_start_indices[span.end.line];
        int last_line_error_len = span.end.column;
        int last_line_len = line_start_indices[span.end.line + 1] - line_start_indices[span.end.line];
        int tail_len = last_line_len - last_line_error_len;

        eprintf(
            "\n%.*s" KEY_UNDERLINE "%.*s\n"
            KEY_RESET "...\n"
            KEY_UNDERLINE "%.*s" KEY_RESET "%.*s\n\n",
            head_len, first_line, first_line_error_len, first_line + head_len,
            last_line_error_len, last_line, tail_len, last_line + last_line_error_len
        );
    } else {
        const char* start_line = source->text.data + line_start_indices[span.start.line];
        int head_len = span.start.column;
        const char* last_line = source->text.data + line_start_indices[span.end.line];
        const char* tail_start = last_line + span.end.column;
        int tail_len = line_start_indices[span.end.line + 1] - span.end.column;

        eprintf(
            "\n%.*s" KEY_UNDERLINE "%.*s" KEY_RESET "%.*s\n\n",
            head_len, start_line,
            (int)span.len, span.data,
            tail_len, tail_start
        );
    }
}

static void default_send(reporter_t* raw_self, error_t error) {
    default_reporter_t* self = (default_reporter_t*)raw_self;
    print_error("%s\n", error.message.data);
    print_error_source(self->source, error.source);
    free_array_buf(error.message);
}

default_reporter_t new_default_reporter(const source_t* source) {
    return (default_reporter_t){
        .reporter = (reporter_t){
            .send = default_send,
            .nerrors = 0
        },
        .source = source
    };
}
