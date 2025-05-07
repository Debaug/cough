#pragma once

#include <stddef.h>

#include "primitives/primitives.h"

typedef enum Marker {
    MARKER_A = 0,
    MARKER_B = 1,
} Marker;

Marker flip_marker(Marker marker);

typedef struct Allocation {
    void* pointer; // null if freed
    usize* subobject_offsets;
    usize subobject_offsets_len;
    Marker marker;
} Allocation;

typedef struct Gc {
    // ArrayBuf /* Allocation */ allocations;
    // ArrayBuf /* usize */ freed;
    // ArrayBuf /* usize */ roots;
    Marker garbage_marker;
} Gc;

Gc new_gc();
