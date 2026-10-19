cmake_minimum_required(VERSION 3.14)
project(kubisat_host_tests CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(frame_crc_tests
    test_frame_crc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/protocol/crc16.cpp
)

target_include_directories(frame_crc_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/protocol
)

enable_testing()
add_test(NAME frame_crc_tests COMMAND frame_crc_tests)
