cmake_minimum_required(VERSION 2.6)
project(griffin)

set(CMAKE_MACOSX_RPATH 0)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/lib)
set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(ENABLE_TESTS 1 CACHE BOOL "Compile tests")
SET(ENABLE_EXAMPLES 1 CACHE BOOL "Compile examples")

file(GLOB_RECURSE source_files "src/*.c")
file(GLOB_RECURSE test_files "tests/*.c")
file(GLOB_RECURSE example_files "examples/*.c")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -Wno-long-long -g -pedantic -std=c11")

include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(griffin ${source_files})

if(${ENABLE_TESTS})
    foreach(test ${test_files})
        get_filename_component(target ${test} NAME_WE)
        add_executable(${target} ${test})
        set_target_properties(${target}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_EXECUTABLE_OUTPUT_PATH}/tests
        )
        add_dependencies(${target} griffin)
        target_link_libraries(${target} griffin)
    endforeach(test)
endif()

if(${ENABLE_EXAMPLES})
    foreach(example ${example_files})
        get_filename_component(target ${example} NAME_WE)
        add_executable(${target} ${example})
        set_target_properties(${target}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_EXECUTABLE_OUTPUT_PATH}/examples
        )
        add_dependencies(${target} griffin)
        target_link_libraries(${target} griffin)
    endforeach(example)
endif()
