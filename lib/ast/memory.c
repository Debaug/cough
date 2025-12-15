#include <string.h>

#include "ast/memory.h"

IMPL_ARRAY_BUF(Allocation)

AstStorage ast_storage_new(void) {
    return (AstStorage){
        ._allocations = array_buf_new(Allocation)(),
    };
}

void ast_storage_free(AstStorage* storage) {
    for (usize i = 0; i < storage->_allocations.len; i++) {
        free(storage->_allocations.data[i]);
    }
    array_buf_free(Allocation)(&storage->_allocations);
}

void ast_store(AstStorage* storage, void* allocation) {
    array_buf_push(Allocation)(&storage->_allocations, allocation);
}
