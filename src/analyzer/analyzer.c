#include "analyzer/analyzer.h"

static const type_info_t primitive_types[] = {
    [TYPE_UNIT] = {
        .size_in_words = 0,
        .storage = TYPE_STORAGE_INLINE,
    },
    [TYPE_BOOL] = {
        .size_in_words = 1,
        .storage = TYPE_STORAGE_INLINE,
    },
    [TYPE_INT] = {
        .size_in_words = 1,
        .storage = TYPE_STORAGE_INLINE,
    },
    [TYPE_FLOAT] = {
        .size_in_words = 1,
        .storage = TYPE_STORAGE_INLINE,
    }
};

analyzer_t new_analyzer(void) {
    return (analyzer_t) {
        .scopes = new_array_buf(),
    };
}

void destroy_analyzer(analyzer_t analyzer) {
    exit(1); // TODO.
}

void analyze(program_t program, analyzer_t* analyzer) {

}
