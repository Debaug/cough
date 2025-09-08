#include <stdlib.h>
#include <string.h>

#include "collections/array.h"
#include "collections/string.h"

StringBuf string_buf_new(void) {
    return (StringBuf){ .data = NULL, .len = 0, .capacity = 0 };
}

void string_buf_free(StringBuf* string) {
    free(string->data);
}

static ArrayBuf(char) to_array(StringBuf* string) {
    return (ArrayBuf(char)){
        .data = string->data,
        .len = string->data ? string->len + 1 : 0,
        .capacity = string->capacity
    };
}

static StringBuf from_array(ArrayBuf(char)* array) {
    return (StringBuf){
        .data = array->data,
        .len = array->data ? array->data - 1 : 0,
        .capacity = array->capacity
    };
}

void string_buf_reserve(StringBuf* string, usize additional) {
    ArrayBuf(char) array = to_array(string);
    array_buf_reserve(char)(&array, additional);
    *string = from_array(&array);
}

void string_buf_push(StringBuf* string, char c) {
    string_buf_extend_slice(string, (String){ .data = &c, .len = 1 });
}

void string_buf_extend(StringBuf* string, char const* s) {
    usize len = strlen(s);
    string_buf_extend_slice(string, (String){ .data = s, .len = len });
}

void string_buf_extend_slice(StringBuf* string, String s) {
    ArrayBuf(char) array = to_array(string);
    if (array.len == 0) {
        array_buf_extend(char)(&array, s.data, s.len);
    } else {
        array.len -= 1;
        array_buf_extend(char)(&array, s.data, s.len);
        array_buf_push(char)(&array, '\0');
    }
    *string = from_array(&array);
}

Errno read_file(FILE* file, ArrayBuf(char)* dst) {
    usize len = 0;
    while (!feof(file)) {
        array_buf_reserve(char)(dst, (len == 0) ? 64 : len);
        usize additional =
            fread(dst->data + dst->len, 1, dst->capacity - dst->len, file);
        if (additional == 0) {
            return errno;
        }
        dst->len += additional;
    }

    return 0;
}
