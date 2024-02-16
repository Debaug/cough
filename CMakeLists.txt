cmake_minimum_required(VERSION 3.27.0)
project(cough VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

find_package(Python REQUIRED)

add_executable(cough "")

add_subdirectory(src)
target_include_directories(cough PRIVATE src build/src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
