cmake_minimum_required(VERSION 3.12)
project(NEO6V2_lib)

add_library(NEO6V2_lib STATIC
    NEO6V2.cpp
    NEO6V2.h
)

target_include_directories(NEO6V2_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NEO6V2_lib PUBLIC
    pico_stdlib
    hardware_uart
)