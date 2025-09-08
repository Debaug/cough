add_subdirectory(primitives)
add_subdirectory(ops)
add_subdirectory(alloc)
add_subdirectory(collections)
add_subdirectory(diagnostics)

add_subdirectory(source)
add_subdirectory(compiler)
add_subdirectory(bytecode)
add_subdirectory(vm)

target_compile_options(libcough PRIVATE "-Wno-gnu-alignof-expression")
