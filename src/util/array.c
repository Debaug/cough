#include <string.h>

#include "util/array.h"

array_buf_t new_array_buf() {
    return (array_buf_t){ .ptr = NULL, .len = 0, .capacity = 0 };
}

void destroy_array_buf(array_buf_t array) {
    free(array.ptr);
}

void array_buf_reserve(array_buf_t* array, size_t additional) {
    size_t new_capacity = array->len + additional;
    if (array->capacity >= new_capacity) {
        return;
    } else if (new_capacity <= 2) {
        new_capacity = 4;
    } else if (new_capacity < 1.5 * array->capacity) {
        new_capacity = 1.5 * array->capacity;
    }

    // safety: new_capacity != 0, see first if statement
    array->ptr = realloc(array->ptr, new_capacity);
    array->capacity = new_capacity;
}

void array_buf_push(array_buf_t* array, const void* data, size_t n) {
    array_buf_reserve(array, n);
    memmove(array->ptr + array->len, data, n);
    array->len += n;
}

read_file_result_t read_file(FILE* file) {
    int error = 0;
    array_buf_t text = new_array_buf();
    while (error == 0) {
        array_buf_reserve(&text, 1);
        size_t n = fread(text.ptr + text.len, 1, text.capacity - text.len, file);
        text.len += n;

        if (feof(file)) {
            break;
        }
        error = ferror(file);
    }
    char zero = '\0';
    array_buf_push(&text, &zero, 1);

    return (read_file_result_t){ .error = error, .text = text };
}
