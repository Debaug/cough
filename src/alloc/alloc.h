#pragma once

#define LAYOUT_OF(elt, ...) sizeof(elt) __VA_OPT__(* __VA_ARGS__), alignof(elt)

typedef size_t handle_t;
#define HANDLE_NULL ((handle_t)(-1))
