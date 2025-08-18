cmake_minimum_required(VERSION 3.12)
project(telemetry_lib)

add_library(telemetry_lib STATIC
    telemetry_manager.cpp
)

target_include_directories(telemetry_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(telemetry_lib PUBLIC
    pico_stdlib
    storage_lib
    PowerManager_lib
    Sensors_lib
    clock_lib
)