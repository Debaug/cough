#pragma once

#include "primitives/primitives.h"

#define LAYOUT_OF(elt, ...) sizeof(elt) __VA_OPT__(* __VA_ARGS__), alignof(elt)

typedef usize Handle;
#define HANDLE_NULL ((Handle)(-1))
