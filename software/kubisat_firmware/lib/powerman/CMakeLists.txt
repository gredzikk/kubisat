cmake_minimum_required(VERSION 3.12)
project(PowerManager_lib)

# Remove add_subdirectory(INA3221)

add_library(PowerManager_lib STATIC
    PowerManager.cpp
    PowerManager.h
    INA3221/INA3221.cpp 
    INA3221/INA3221.h    
)

target_include_directories(PowerManager_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/eventman
)

target_link_libraries(PowerManager_lib PUBLIC
    pico_stdlib
    hardware_i2c
    storage_lib
    eventman_lib
)