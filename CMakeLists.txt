cmake_minimum_required(VERSION 3.27.0)
project(cough VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -fsanitize=address,undefined")

add_library(libcough "")
add_subdirectory(lib)
target_include_directories(libcough PRIVATE lib)
set_target_properties(libcough PROPERTIES OUTPUT_NAME cough)

add_executable(cough "")
add_subdirectory(app)
target_include_directories(cough PRIVATE app lib)
target_link_libraries(cough PRIVATE libcough)

include(CTest)
enable_testing()
set(CTEST_OUTPUT_ON_FAILURE 1)
add_subdirectory(tests/tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
