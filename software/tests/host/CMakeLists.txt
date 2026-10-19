cmake_minimum_required(VERSION 3.14)
project(kubisat_host_tests CXX)
set(CMAKE_CXX_STANDARD 17)

set(KUBISAT_PROTOCOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../common/protocol)

add_executable(frame_crc_tests
    test_frame_crc.cpp
    ${KUBISAT_PROTOCOL_DIR}/crc16.cpp
)

target_include_directories(frame_crc_tests PRIVATE ${KUBISAT_PROTOCOL_DIR})

add_executable(ground_bridge_tests
    test_ground_bridge.cpp
)

target_include_directories(ground_bridge_tests PRIVATE ${KUBISAT_PROTOCOL_DIR})

add_executable(frame_decode_vectors
    test_frame_decode_vectors.cpp
    ${KUBISAT_PROTOCOL_DIR}/crc16.cpp
)

target_include_directories(frame_decode_vectors PRIVATE ${KUBISAT_PROTOCOL_DIR})

enable_testing()
add_test(NAME frame_crc_tests COMMAND frame_crc_tests)
add_test(NAME ground_bridge_tests COMMAND ground_bridge_tests)
add_test(NAME frame_decode_vectors COMMAND frame_decode_vectors)
