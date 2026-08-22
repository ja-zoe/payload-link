add_executable(test_crc16 test_crc16.c)
target_link_libraries(test_crc16 PRIVATE payload_link)
add_test(NAME crc16 COMMAND test_crc16)

add_executable(test_roundtrip test_roundtrip.c)
target_link_libraries(test_roundtrip PRIVATE payload_link)
add_test(NAME roundtrip COMMAND test_roundtrip)

add_executable(test_resync test_resync.c)
target_link_libraries(test_resync PRIVATE payload_link)
add_test(NAME resync COMMAND test_resync)
