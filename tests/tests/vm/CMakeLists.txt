target_sources(testcough PRIVATE
    common.h common.c
    function_call.h function_call.c
)

add_test(NAME vm/function_call COMMAND $<TARGET_FILE:testcough> vm/function_call)
