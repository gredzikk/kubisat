# lib/comms/commands/CMakeLists.txt
cmake_minimum_required(VERSION 3.12)
project(commands_lib)

add_library(commands_lib STATIC
    commands.cpp
    clock_commands.cpp
    diagnostic_commands.cpp
    gps_commands.cpp
    event_commands.cpp
    telemetry_commands.cpp
)

target_include_directories(commands_lib PUBLIC
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
    ${CMAKE_SOURCE_DIR}/lib/sensors
    ${CMAKE_SOURCE_DIR}/lib/telemetry
)

target_link_libraries(commands_lib PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_spi
    hardware_flash
    blockdevice_flash
    blockdevice_heap
    blockdevice_loopback
    blockdevice_sd
    filesystem_fat
    filesystem_littlefs
    filesystem_vfs
    storage_lib
    Sensors_lib
)