
add_library(telemetry_lib STATIC
    telemetry_manager.cpp
)

target_include_directories(telemetry_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/comms
    ${CMAKE_SOURCE_DIR}/../common/protocol
)

target_link_libraries(telemetry_lib PUBLIC
    pico_stdlib
    storage_lib
    PowerManager_lib
    Sensors_lib
    clock_lib
)