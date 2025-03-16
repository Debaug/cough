target_sources(cough PRIVATE main.c)

add_subdirectory(alloc)
add_subdirectory(diagnostic)

add_subdirectory(text)
add_subdirectory(tokens)
add_subdirectory(ast)
add_subdirectory(bytecode)
add_subdirectory(vm)

target_compile_options(cough PRIVATE "-Wno-gnu-alignof-expression")
