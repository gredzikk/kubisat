cmake_minimum_required(VERSION 3.12)
project(eventman_lib)

add_library(eventman_lib STATIC
    event_manager.cpp
)

target_include_directories(eventman_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/comms
    ${CMAKE_SOURCE_DIR}/lib/powerman
    ${CMAKE_SOURCE_DIR}/lib/powerman/INA3221
    ${PICO_SDK_PATH}/src/common/hardware_i2c/include
    ${CMAKE_SOURCE_DIR}/lib/eventman
    ${CMAKE_SOURCE_DIR}/lib/clock
    ${CMAKE_SOURCE_DIR}/lib/location
    ${CMAKE_SOURCE_DIR}/lib/storage
)

target_link_libraries(eventman_lib PUBLIC
    pico_stdlib
    hardware_sync
    hardware_flash
    PowerManager_lib
)