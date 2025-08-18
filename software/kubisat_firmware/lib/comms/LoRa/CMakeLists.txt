cmake_minimum_required(VERSION 3.12)

project(LoRa_pico_lib)

add_library(LoRa_pico_lib 
    LoRa-RP2040.cpp 
    LoRa-RP2040.h
)


target_link_libraries(LoRa_pico_lib PUBLIC
    pico_stdlib 
    hardware_spi 
    hardware_interp 
)

target_include_directories(LoRa_pico_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)
