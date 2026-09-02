cmake_minimum_required(VERSION 3.13)
project(payload_link C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PL_WARN_FLAGS -Wall -Wextra -Wpedantic -pedantic-errors)

add_library(payload_link STATIC
    src/crc16.c
    src/pack.c
    src/framer.c
    src/frame.c
)
target_include_directories(payload_link PUBLIC include)
target_compile_options(payload_link PRIVATE ${PL_WARN_FLAGS})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_executable(payload_link_example examples/consumer.c)
    target_link_libraries(payload_link_example PRIVATE payload_link)
    target_compile_options(payload_link_example PRIVATE ${PL_WARN_FLAGS})
endif()

# Build tests for standalone builds and when a parent project enables them.
# These are ordinary CTest executables with no external test dependency.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR ENABLE_UNIT_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
