cmake_minimum_required(VERSION 3.12)

project(LoRa_pico_lib)

add_library(LoRa_pico_lib 
    LoRa-RP2040.cpp 
    LoRa-RP2040.h
)

add_library(LoRa_print 
    Print.h 
    Print.cpp
)

target_link_libraries(LoRa_pico_lib PUBLIC
    pico_stdlib 
    hardware_spi 
    hardware_interp 
    LoRa_print
)

target_include_directories(LoRa_pico_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

 pico_enable_stdio_usb(LoRa_pico_lib 1)
 pico_enable_stdio_uart(LoRa_pico_lib 0)