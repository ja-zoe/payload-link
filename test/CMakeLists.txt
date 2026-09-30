add_executable(test_crc16 test_crc16.c)
target_link_libraries(test_crc16 PRIVATE payload_link)
add_test(NAME crc16 COMMAND test_crc16)

add_executable(test_roundtrip test_roundtrip.c)
target_link_libraries(test_roundtrip PRIVATE payload_link)
add_test(NAME roundtrip COMMAND test_roundtrip)

add_executable(test_resync test_resync.c)
target_link_libraries(test_resync PRIVATE payload_link)
add_test(NAME resync COMMAND test_resync)

add_executable(test_framer_config test_framer_config.c)
target_link_libraries(test_framer_config PRIVATE payload_link)
add_test(NAME framer_config COMMAND test_framer_config)

# The C++ test proves the public headers can be consumed from C++ (for example
# by the NOS3 PayOBC simulator). Skip it when no C++ compiler is available.
include(CheckLanguage)
check_language(CXX)
if(CMAKE_CXX_COMPILER)
    enable_language(CXX)
    add_executable(test_cxx_linkage test_cxx_linkage.cpp)
    target_link_libraries(test_cxx_linkage PRIVATE payload_link)
    set_target_properties(test_cxx_linkage PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)
    target_compile_options(test_cxx_linkage PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors)
    add_test(NAME cxx_linkage COMMAND test_cxx_linkage)
endif()
