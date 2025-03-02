#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdarg.h>

#include "alloc/array.h"
#include "diagnostic/diagnostic.h"

void raw_array_buf_reserve(
    void** restrict data,
    size_t len,
    size_t* restrict capacity_bytes,
    size_t additional,
    size_t element_size
) {
    size_t target_capacity = (len + additional) * element_size;
    if (target_capacity <= *capacity_bytes) {
        return;
    }

    if (target_capacity <= 3) {
        target_capacity = 4;
    } else if (target_capacity <= 1.5 * *capacity_bytes) {
        target_capacity = 1.5 * *capacity_bytes;
    }
    *data = realloc(*data, target_capacity);
    if (data == NULL) {
        print_errno();
        exit(EXIT_FAILURE);
    }

    *capacity_bytes = target_capacity;
}

void raw_array_buf_extend(
    void** restrict array_data,
    size_t* restrict array_len,
    size_t* restrict array_capacity_bytes,
    void* src_data,
    size_t src_len,
    size_t element_size
) {
    raw_array_buf_reserve(
        array_data,
        *array_len,
        array_capacity_bytes,
        src_len,
        element_size
    );
    memmove(*array_data + *array_len * element_size, src_data, src_len * element_size);
    *array_len += src_len;
}

void raw_array_buf_pop(
    void* array_data,
    size_t* restrict array_len,
    void* dst_data,
    size_t dst_len,
    size_t element_size
) {
    *array_len -= dst_len;
    if (dst_data == NULL) return;
    memmove(dst_data, array_data + *array_len * element_size, dst_len * element_size);
}

string_buf_t format(const char* restrict fmt, ...) {
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);

    string_buf_t buf = new_array_buf();
    int bufsz = vsnprintf(NULL, 0, fmt, args1);
    if (bufsz < 0) {
        print_system_error("failed to format string (please report)");
        exit(EXIT_FAILURE);
    }
    array_buf_reserve(&buf, bufsz + 1, char);

    if (vsprintf(buf.data, fmt, args2) < 0) {
        print_system_error("failed to format string (please report)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

bool string_views_eq(string_view_t a, string_view_t b) {
    if (a.len != b.len) {
        return false;
    }
    return strncmp(a.data, b.data, a.len) == 0;
}

errno_t read_file(FILE* file, string_buf_t* dst) {
    while (!feof(file)) {
        array_buf_reserve(dst, 1, char);
        size_t nread =
            fread(dst->data + dst->len, 1, dst->capacity_bytes - dst->len, file);
        dst->len += nread;
        if (nread == 0) {
            return errno;
        }
    }
    return SUCCESS;
}
