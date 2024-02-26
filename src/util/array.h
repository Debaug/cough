#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "diagnostic/diagnostic.h"

#define array_buf_t(T) struct array_buf__ ## T {  \
    T* data;                                    \
    size_t len;                                 \
    size_t capacity_bytes;                      \
}

#define new_array_buf(T) (      \
    (struct array_buf__ ## T) { \
        .data = NULL,           \
        .len = 0,               \
        .capacity_bytes = 0     \
    }                           \
)

#define free_array_buf(array) free((array).data)

void raw_array_buf_reserve(
    void** restrict data,
    size_t len,
    size_t* restrict capacity_bytes,
    size_t additional,
    size_t element_size
);

#define array_buf_reserve(array, additional, T) \
    raw_array_buf_reserve(                      \
        (void**)&(array)->data,                 \
        (array)->len,                           \
        &(array)->capacity_bytes,               \
        (additional),                           \
        sizeof(T)                               \
    )

void raw_array_buf_extend(
    void** restrict array_data,
    size_t* restrict array_len,
    size_t* restrict array_capacity_bytes,
    void* src_data,
    size_t src_len,
    size_t element_size
);

#define array_buf_push(array, elt)  \
    raw_array_buf_extend(           \
        (void**)&(array)->data,     \
        &(array)->len,              \
        &(array)->capacity_bytes,   \
        &(elt),                     \
        1,                          \
        sizeof(elt)                 \
    )

#define array_buf_extend(array, src, length, T) \
    raw_array_buf_extend(                       \
        (void**)&(array)->data,                 \
        &(array)->len,                          \
        &(array)->capacity_bytes,               \
        (src),                                  \
        (length),                               \
        sizeof(T)                               \
    )

void raw_array_buf_pop(
    void* array_data,
    size_t* restrict array_len,
    void* dst_data,
    size_t dst_len,
    size_t element_size
);

#define array_buf_pop_array(array, dst, T, n)   \
    raw_array_buf_pop(                          \
        (array)->data,                          \
        &(array)->len,                          \
        (dst),                                  \
        n,                                      \
        sizeof(T)                               \
    )

#define array_buf_pop(array, dst, T) array_buf_pop_array(array, dst, T, 1)

typedef array_buf_t(char) string_buf_t;

typedef struct string_view {
    const char* ptr;
    size_t len;
} string_view_t;

#define STRING_VIEW(src) (string_view_t){ .ptr = src.ptr, .len = src.len }

errno_t read_file(FILE* file, string_buf_t* dst);
