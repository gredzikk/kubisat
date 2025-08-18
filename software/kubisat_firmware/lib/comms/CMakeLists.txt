# lib/comms/CMakeLists.txt
cmake_minimum_required(VERSION 3.12)
project(Comms_lib)

add_subdirectory(LoRa)
add_subdirectory(commands)

add_library(Comms_lib STATIC
    frame.cpp
    send.cpp
    receive.cpp
    communication.cpp
    utils_converters.cpp
)

target_include_directories(Comms_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/powerman
    ${CMAKE_SOURCE_DIR}/lib/eventman
)

target_link_libraries(Comms_lib PUBLIC
    LoRa_pico_lib
    commands_lib
    pico_stdlib
)