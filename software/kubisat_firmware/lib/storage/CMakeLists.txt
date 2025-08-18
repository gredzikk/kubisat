cmake_minimum_required(VERSION 3.12)
project(storage_lib)

add_library(storage_lib STATIC
    storage.cpp
    storage.h
)

target_include_directories(storage_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/storage
    ${CMAKE_SOURCE_DIR}/lib/storage/pico-vfs
    ${CMAKE_SOURCE_DIR}/lib/storage/pico-vfs/include/blockdevice
    ${CMAKE_SOURCE_DIR}/lib/storage/pico-vfs/vendor/littlefs
)

target_link_libraries(storage_lib PUBLIC
    pico_stdlib
    hardware_flash
    blockdevice_flash
    blockdevice_heap
    blockdevice_loopback
    blockdevice_sd
    filesystem_fat
    filesystem_littlefs
    filesystem_vfs
)