#pragma once

#include <stdbool.h>

#define DEFINE_RESULT(name, t, e)   \
    typedef struct name {           \
        bool is_ok: 1;              \
        union {                     \
            t ok;                   \
            e error;                \
        };                          \
    } name##_t;

#define RESULT_OK(name, ...) (name){ .is_ok = true, .ok = __VA_ARGS__ }
#define RESULT_ERROR(name, ...) (name) { .is_ok = false, .error = __VA_ARGS__ }

#define CAST_ERROR(type, result) \
    (type){ .is_ok = false, .error = result.error }
