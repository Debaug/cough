#pragma once

#include <stddef.h>

typedef enum marker {
    MARKER_A = 0,
    MARKER_B = 1,
} marker_t;

marker_t flip_marker(marker_t marker);

typedef struct allocation {
    void* pointer; // null if freed
    size_t* subobject_offsets;
    size_t subobject_offsets_len;
    marker_t marker;
} allocation_t;

typedef struct gc {
    // array_buf_t /* allocation_t */ allocations;
    // array_buf_t /* size_t */ freed;
    // array_buf_t /* size_t */ roots;
    marker_t garbage_marker;
} gc_t;

gc_t new_gc();
