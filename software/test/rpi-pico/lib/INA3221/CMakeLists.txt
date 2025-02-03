cmake_minimum_required(VERSION 3.12)
project(INA3221_lib)

add_library(INA3221_lib STATIC
    INA3221.cpp
    INA3221.h
)

target_include_directories(INA3221_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(INA3221_lib PUBLIC
    pico_stdlib
    hardware_i2c
)