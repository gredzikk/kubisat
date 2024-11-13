cmake_minimum_required(VERSION 3.12)
project(DS3231_lib)

add_library(DS3231_lib STATIC
    DS3231.cpp
    DS3231.h
)

target_include_directories(DS3231_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DS3231_lib PUBLIC
    pico_stdlib
    hardware_i2c
)