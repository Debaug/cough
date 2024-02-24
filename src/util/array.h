#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "diagnostic/diagnostic.h"

typedef struct array_buf {
    void* ptr;
    size_t len;
    size_t capacity;
} array_buf_t;

typedef array_buf_t string_buf_t;

typedef struct string_view {
    const char* ptr;
    size_t len;
} string_view_t;

#define ARRAY_VIEW(src) (array_view_t){ .ptr = src.ptr, .len = src.len }
#define STRING_VIEW(src) (string_view_t){ .ptr = src.ptr, .len = src.len }

array_buf_t new_array_buf(void);
void destroy_array_buf(array_buf_t array);
void array_buf_reserve(array_buf_t* array, size_t additional);
void array_buf_push(array_buf_t* array, const void* data, size_t n);
void array_buf_pop(array_buf_t* array, void* data, size_t n);
void array_buf_align(array_buf_t* array, size_t alignment);
errno_t read_file(FILE* file, string_buf_t* dst);
