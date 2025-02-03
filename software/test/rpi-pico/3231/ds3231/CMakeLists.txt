cmake_minimum_required(VERSION 3.12)

project(ds3231_library)

add_library(ds3231 STATIC
    ds3231.cpp
    ds3231.h
)

target_include_directories(ds3231 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ds3231 pico_stdlib hardware_i2c)