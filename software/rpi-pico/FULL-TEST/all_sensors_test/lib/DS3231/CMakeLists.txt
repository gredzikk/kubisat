cmake_minimum_required(VERSION 3.12)
project(DS3231_lib)
add_library(DS3231_lib STATIC
    DS3231.cpp
    DS3231.h
    DS3231_WRAPPER.cpp
    DS3231_WRAPPER.h
)
target_include_directories(DS3231_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}    # For ISensor.h access
)
target_link_libraries(DS3231_lib PUBLIC
    pico_stdlib
    hardware_i2c
)