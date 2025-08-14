#pragma once

#include <stdalign.h>

#include "primitives/primitives.h"

const void* align_down(const void* ptr, usize alignment);
void* align_down_mut(void* ptr, usize alignment);
usize align_down_size(usize size, usize alignment);

const void* align_up(const void* ptr, usize alignment);
void* align_up_mut(void* ptr, usize alignment);
usize align_up_size(usize size, usize alignment);

typedef struct Layout {
    usize size;
    usize alignment;
} Layout;

#define LAYOUT_OF(elt, ...) sizeof(elt) __VA_OPT__(* __VA_ARGS__), alignof(elt)

typedef usize Handle;
#define HANDLE_NULL ((Handle)(-1))
