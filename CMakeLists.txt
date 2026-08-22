cmake_minimum_required(VERSION 3.13)
project(payload_link C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PL_WARN_FLAGS -Wall -Wextra -Wpedantic -pedantic-errors)

add_library(payload_link STATIC
    src/crc16.c
    src/pack.c
    src/frame.c
)
target_include_directories(payload_link PUBLIC include)
target_compile_options(payload_link PRIVATE ${PL_WARN_FLAGS})

# Only build tests when this is the top-level project — not when a consumer
# add_subdirectory()s this into their own build.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    enable_testing()
    add_subdirectory(test)
endif()
