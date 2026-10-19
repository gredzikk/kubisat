# lib/comms/CMakeLists.txt

set(KUBISAT_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../common")
add_subdirectory(${KUBISAT_COMMON_DIR}/LoRa ${CMAKE_BINARY_DIR}/common_LoRa)
target_include_directories(LoRa_pico_lib PUBLIC ${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(commands)

add_library(Comms_lib STATIC
    frame.cpp
    send.cpp
    receive.cpp
    communication.cpp
    utils_converters.cpp
    ${KUBISAT_COMMON_DIR}/protocol/crc16.cpp
)

target_include_directories(Comms_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/powerman
    ${CMAKE_SOURCE_DIR}/lib/eventman
    ${KUBISAT_COMMON_DIR}/LoRa
    ${KUBISAT_COMMON_DIR}/protocol
)

target_link_libraries(Comms_lib PUBLIC
    LoRa_pico_lib
    commands_lib
    pico_stdlib
)
