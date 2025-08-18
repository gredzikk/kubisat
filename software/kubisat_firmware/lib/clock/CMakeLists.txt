cmake_minimum_required(VERSION 3.12)
project(clock_lib)
add_library(clock_lib STATIC
    DS3231.cpp
    DS3231.h
)
target_include_directories(clock_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/eventman
    ${CMAKE_SOURCE_DIR}/lib/powerman
    ${CMAKE_SOURCE_DIR}/lib/powerman/INA3221
    ${CMAKE_SOURCE_DIR}/lib/storage
    ${CMAKE_SOURCE_DIR}/lib/location/
    ${CMAKE_SOURCE_DIR}/lib/location/NMEA
)
target_link_libraries(clock_lib PUBLIC
    pico_stdlib
    hardware_i2c
    hardware_flash
    blockdevice_flash
    blockdevice_heap
    blockdevice_loopback
    blockdevice_sd
    filesystem_fat
    filesystem_littlefs
    filesystem_vfs
)