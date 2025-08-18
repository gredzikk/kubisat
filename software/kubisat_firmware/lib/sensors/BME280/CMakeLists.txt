cmake_minimum_required(VERSION 3.12)
project(BME280_lib)
add_library(BME280_lib STATIC
    BME280.cpp
    BME280.h
    BME280_WRAPPER.cpp
    BME280_WRAPPER.h
)
target_include_directories(BME280_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/sensors
    ${CMAKE_SOURCE_DIR}/lib
)
target_link_libraries(BME280_lib PUBLIC
    pico_stdlib
    hardware_i2c
)