#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "primitives/primitives.h"

#define ArrayBuf(T) struct ArrayBuf_ ## T { \
    T* data;                                \
    usize len;                              \
    usize capacity_bytes;                   \
}

typedef ArrayBuf(u8) U8ArrayBuf;
typedef ArrayBuf(u16) U16ArrayBuf;
typedef ArrayBuf(u32) U32ArrayBuf;
typedef ArrayBuf(u64) U64ArrayBuf;
typedef ArrayBuf(usize) UsizeArrayBuf;
typedef ArrayBuf(umax) UmaxArrayBuf;

typedef ArrayBuf(i8) I8ArrayBuf;
typedef ArrayBuf(i16) I16ArrayBuf;
typedef ArrayBuf(i32) I32ArrayBuf;
typedef ArrayBuf(i64) I64ArrayBuf;
typedef ArrayBuf(imax) ImaxArrayBuf;

#define new_array_buf(...)                              \
    __VA_OPT__((struct ArrayBuf_ ## __VA_ARGS__)) {     \
        .data = NULL,                                   \
        .len = 0,                                       \
        .capacity_bytes = 0                             \
    }

#define free_array_buf(array) free((array).data)

void raw_array_buf_reserve(
    void** restrict data,
    usize len,
    usize* restrict capacity_bytes,
    usize additional,
    usize element_size
);

#define array_buf_reserve(array, additional)    \
    raw_array_buf_reserve(                      \
        (void**)&(array)->data,                 \
        (array)->len,                           \
        &(array)->capacity_bytes,               \
        (additional),                           \
        sizeof(*(array)->data)                  \
    )

void raw_array_buf_extend(
    void** restrict array_data,
    usize* restrict array_len,
    usize* restrict array_capacity_bytes,
    void* src_data,
    usize src_len,
    usize element_size
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

#define array_buf_extend(array, src, length)    \
    raw_array_buf_extend(                       \
        (void**)&(array)->data,                 \
        &(array)->len,                          \
        &(array)->capacity_bytes,               \
        (src),                                  \
        (length),                               \
        sizeof(*(array)->data)                  \
    )

void raw_array_buf_pop(
    void* array_data,
    usize* restrict array_len,
    void* dst_data,
    usize dst_len,
    usize element_size
);

#define array_buf_pop_array(array, dst, n)      \
    raw_array_buf_pop(                          \
        (array)->data,                          \
        &(array)->len,                          \
        (dst),                                  \
        n,                                      \
        sizeof(*((array)->data))                \
    )

#define array_buf_pop(array, dst) array_buf_pop_array(array, dst, 1)

typedef const char* String;

typedef ArrayBuf(char) StringBuf;

StringBuf format(const char* restrict fmt, ...);

typedef struct StringView {
    const char* data;
    usize len;
} StringView;

#define STRING_VIEW(src) (StringView){ .data = src.data, .len = src.len }
#define STRING_FMT(str) (int)(str).len, (str).data

bool string_views_eq(StringView a, StringView b);

typedef int Errno; // FIXME: duplicate declaration
Errno read_file(FILE* file, StringBuf* dst);
