cmake_minimum_required(VERSION 3.12)
project(SD_MICRO_lib)

add_library(SD_MICRO_lib STATIC
    SD_MICRO.cpp
    SD_MICRO.h
)

target_include_directories(SD_MICRO_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SD_MICRO_lib PUBLIC
    pico_stdlib
    hardware_spi
)