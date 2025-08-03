target_sources(testcough PRIVATE
    common.h common.c
    tests.h
    function_call.c
    conditional.c
)

add_test(NAME vm/function_call COMMAND $<TARGET_FILE:testcough> vm/function_call)
add_test(NAME vm/conditional COMMAND $<TARGET_FILE:testcough> vm/conditional)
