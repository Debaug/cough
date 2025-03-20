#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdarg.h>

#include "alloc/array.h"
#include "diagnostics/diagnostics.h"

void raw_array_buf_reserve(
    void** restrict data,
    usize len,
    usize* restrict capacity_bytes,
    usize additional,
    usize element_size
) {
    usize target_capacity = (len + additional) * element_size;
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
    usize* restrict array_len,
    usize* restrict array_capacity_bytes,
    void* src_data,
    usize src_len,
    usize element_size
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
    usize* restrict array_len,
    void* dst_data,
    usize dst_len,
    usize element_size
) {
    *array_len -= dst_len;
    if (dst_data == NULL) return;
    memmove(dst_data, array_data + *array_len * element_size, dst_len * element_size);
}

StringBuf format(const char* restrict fmt, ...) {
    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);

    StringBuf buf = new_array_buf();
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

bool string_views_eq(StringView a, StringView b) {
    if (a.len != b.len) {
        return false;
    }
    return strncmp(a.data, b.data, a.len) == 0;
}

Errno read_file(FILE* file, StringBuf* dst) {
    while (!feof(file)) {
        array_buf_reserve(dst, 1, char);
        usize nread =
            fread(dst->data + dst->len, 1, dst->capacity_bytes - dst->len, file);
        dst->len += nread;
        if (nread == 0) {
            return errno;
        }
    }
    return SUCCESS;
}
