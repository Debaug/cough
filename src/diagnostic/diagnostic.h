#pragma once

#include <stdio.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define KEY(n) "\x1B[" n "m"
#define KEY_RESET KEY("0")
#define KEY_BOLD KEY("1")
#define KEY_RED KEY("31")
#define KEY_BRIGHT_RED KEY("91")
#define KEY_BRIGHT_CYAN KEY("96")

#define report(format, ...) eprintf(format "\n" __VA_OPT__(,) __VA_ARGS__)

#define report_system_error(format, ...)                    \
    report(                                                 \
        KEY_BOLD KEY_RED "system error" KEY_RESET ": "      \
        format __VA_OPT__(,) __VA_ARGS__                    \
    )

#define report_error(format, ...)                           \
    report(                                                 \
        KEY_BOLD KEY_BRIGHT_RED "error" KEY_RESET ": "      \
        format __VA_OPT__(,) __VA_ARGS__                    \
    )

#define report_text_error(text_view, format, ...)   \
    report_error(                                   \
        "%zu:%zu-%zu:%zu: " format,                 \
        (text_view).start.column + 1,               \
        (text_view).start.line + 1,                 \
        (text_view).end.column + 1,                 \
        (text_view).end.line + 1                    \
        __VA_OPT__(,) __VA_ARGS__                   \
    )

typedef int errno_t;
#define SUCCESS ((errno_t)0)

void report_errno(void);
