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

typedef int errno_t;
#define SUCCESS 0

void report_errno(void);
