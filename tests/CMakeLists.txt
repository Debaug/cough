target_sources(testcough PRIVATE test.c)

add_test(NAME basic COMMAND $<TARGET_FILE:testcough>)

add_subdirectory(vm)
