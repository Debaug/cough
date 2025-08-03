target_sources(testcough PRIVATE
    common.h common.c
    tests.h
    function_call.c
    conditional.c
    fibonacci.c
)

cough_test(vm/function_call)
cough_test(vm/conditional)
cough_test(vm/fibonacci)
