#include <string.h>
#include <stdbool.h>
#include <errno.h>

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

void array_buf_pop(array_buf_t* array, void* data, size_t n) {
    array->len -= n;
    if (data == NULL) return;
    memmove(data, array->ptr + array->len, n);
}

void array_buf_align(array_buf_t* array, size_t alignment) {
    array->ptr = __builtin_align_up(array->ptr, alignment);
}

errno_t read_file(FILE* file, string_buf_t* dst) {
    while (true) {
        array_buf_reserve(dst, 1);
        size_t n = fread(dst->ptr + dst->len, 1, dst->capacity - dst->len, file);
        dst->len += n;

        if (feof(file)) {
            break;
        }
        if (ferror(file)) {
            return errno;
        }
    }
    return 0;
}
