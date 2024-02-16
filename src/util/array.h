#pragma once

#include <stdlib.h>
#include <stdio.h>

typedef struct array_buf {
    void* ptr;
    size_t len;
    size_t capacity;
} array_buf_t;

typedef struct string_view {
    const char* ptr;
    size_t len;
} string_view_t;

#define ARRAY_VIEW(src) (array_view_t){ .ptr = src.ptr, .len = src.len }
#define STRING_VIEW(src) (string_view_t){ .ptr = src.ptr, .len = src.len }

typedef struct read_file_result {
    int error;
    array_buf_t text;
} read_file_result_t;

array_buf_t new_array_buf(void);
void destroy_array_buf(array_buf_t array);
void array_buf_reserve(array_buf_t* array, size_t additional);
void array_buf_push(array_buf_t* array, const void* data, size_t n);
void array_buf_pop(array_buf_t* array, void* data, size_t n);
read_file_result_t read_file(FILE* file);
