#include "vm/gc.h"

Gc new_gc() {
    return (Gc){
        // .allocations = new_array_buf(),
        // .freed = new_array_buf(),
        // .roots = new_array_buf(),
        .garbage_marker = MARKER_A,
    };
}

Marker flip_marker(Marker marker) {
    return marker ^ 1;
}
