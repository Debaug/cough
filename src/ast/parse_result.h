#pragma once

#include "util/result.h"

typedef enum parse_error {
    PARSE_SUCCESS,
    PARSE_ERROR,
} parse_error_t;

#define DEFINE_PARSE_RESULT(name, T) DEFINE_RESULT(name, T, parse_error_t)
