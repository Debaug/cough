target_sources(cough PRIVATE main.c)

add_subdirectory(common)
add_subdirectory(alloc)
add_subdirectory(diagnostics)

add_subdirectory(text)
add_subdirectory(compiler)
add_subdirectory(bytecode)
add_subdirectory(vm)

target_compile_options(cough PRIVATE "-Wno-gnu-alignof-expression")
