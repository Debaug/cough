#include <string.h>
#include <errno.h>

#include "diagnostic/diagnostic.h"

void print_errno(void) {
    print_system_error("%s (errno %d)", strerror(errno), errno);
}

static void print_error_source(const Source* source, TextView span) {
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
            "at %s:%zu.%zu-%zu\n",
            source->path,
            span.start.line + 1,
            span.start.column + 1,
            span.end.column + 1
        );
    } else {
        eprintf(
            "at %s:%zu.%zu-%zu.%zu\n",
            source->path,
            span.start.line + 1,
            span.start.column + 1,
            span.end.line + 1,
            span.end.column + 1
        );
    }

    usize* line_start_indices = source->line_start_indices.data;
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
            "%.*s" KEY_UNDERLINE KEY_BOLD "%.*s\n"
            KEY_RESET "...\n"
            KEY_UNDERLINE KEY_BOLD "%.*s" KEY_RESET "%.*s\n\n",
            head_len, first_line, first_line_error_len, first_line + head_len,
            last_line_error_len, last_line, tail_len, last_line + last_line_error_len
        );
    } else {
        const char* start_line = source->text.data + line_start_indices[span.start.line];
        int head_len = span.start.column;
        const char* last_line = source->text.data + line_start_indices[span.end.line];
        const char* tail_start = last_line + span.end.column;
        int tail_len = line_start_indices[span.end.line + 1] - span.end.index;

        eprintf(
            "%.*s" KEY_UNDERLINE KEY_BOLD "%.*s" KEY_RESET "%.*s\n\n",
            head_len, start_line,
            (int)span.len, span.data,
            tail_len, tail_start
        );
    }
}

static void default_send(Reporter* raw_self, Error* error) {
    DefaultReporter* self = (DefaultReporter*)raw_self;
    print_error("%s\n", error->message.data);
    print_error_source(self->source, error->source);
    free_array_buf(error->message);
}

DefaultReporter new_default_reporter(const Source* source) {
    return (DefaultReporter){
        .reporter = (Reporter){
            .send = default_send,
            .nerrors = 0
        },
        .source = source
    };
}
