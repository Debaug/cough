#include "vm/gc.h"

gc_t new_gc() {
    return (gc_t){
        // .allocations = new_array_buf(),
        // .freed = new_array_buf(),
        // .roots = new_array_buf(),
        .garbage_marker = MARKER_A,
    };
}

marker_t flip_marker(marker_t marker) {
    return marker ^ 1;
}
