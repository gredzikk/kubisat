cmake_minimum_required(VERSION 3.12)
project(MPU6050_lib)

add_library(MPU6050_lib STATIC
    MPU6050.cpp
    MPU6050.h
)

target_include_directories(MPU6050_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MPU6050_lib PUBLIC
    pico_stdlib
    hardware_i2c
)