cmake_minimum_required(VERSION 3.27.0)
project(cough VERSION 0.1.0 LANGUAGES C)

add_library(libcough "")
add_subdirectory(lib)
target_include_directories(libcough PRIVATE lib)

add_executable(cough "")
add_subdirectory(app)
target_include_directories(cough PRIVATE app lib)
target_link_libraries(cough PRIVATE libcough)

include(CTest)
enable_testing()
add_executable(testcough "")
add_subdirectory(tests)
target_include_directories(testcough PRIVATE tests lib)
target_link_libraries(testcough PRIVATE libcough)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
