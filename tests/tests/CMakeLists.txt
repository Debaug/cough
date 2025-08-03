function(cough_test name)
    add_test(NAME ${name} COMMAND $<TARGET_FILE:testcough> ${name})
endfunction(cough_test)

add_subdirectory(vm)
