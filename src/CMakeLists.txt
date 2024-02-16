target_sources(cough PRIVATE main.c)

add_subdirectory(util)
add_subdirectory(text)
add_subdirectory(scanner)
add_subdirectory(ast)
add_subdirectory(compiler)
add_subdirectory(vm)
