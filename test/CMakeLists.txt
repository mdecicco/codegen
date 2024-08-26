cmake_minimum_required(VERSION 3.0.0)
project(codegen_test VERSION 0.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(
    codegen_test_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(${codegen_test_include_dirs})

file(GLOB all_sources "./*.cpp")
add_executable(codegen_test ${all_sources})
target_link_libraries(codegen_test codegen)