cmake_minimum_required(VERSION 3.12)
project(BH1750_lib)
add_library(BH1750_lib STATIC
    BH1750.cpp
    BH1750.h
    BH1750_WRAPPER.cpp
    BH1750_WRAPPER.h
)
target_include_directories(BH1750_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/sensors
)
target_link_libraries(BH1750_lib PUBLIC
    pico_stdlib
    hardware_i2c
)