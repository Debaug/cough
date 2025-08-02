add_subdirectory(primitives)
add_subdirectory(alloc)
add_subdirectory(diagnostics)

add_subdirectory(text)
add_subdirectory(compiler)
add_subdirectory(bytecode)
add_subdirectory(vm)

target_compile_options(libcough PRIVATE "-Wno-gnu-alignof-expression")
