#include <assert.h>

#include "collections/array.h"

typedef struct Foo {
    i32 id;
} Foo;
DECL_ARRAY_BUF(Foo);
IMPL_ARRAY_BUF(Foo);

int main(int argc, char const* argv[]) {
    ArrayBuf(Foo) foos = array_buf_new(Foo)();
    assert(foos.len == 0);

    array_buf_push(Foo)(&foos, (Foo){ 0 });
}
