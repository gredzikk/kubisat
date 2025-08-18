# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Main CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

# Basic project setup
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_CXX_ENABLE_RTTI 1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wunused-function -Wunused-variable -Wunused -Wunreachable-code")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wunused-function -Wunused-variable -Wunused -Wunreachable-code")

# VS Code Pico extension configuration (only needed once)
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# SDK initialization
include(pico_sdk_import.cmake)
project(main C CXX ASM)
pico_sdk_init()

add_subdirectory(lib/sensors)
add_subdirectory(lib/clock)
add_subdirectory(lib/powerman)
add_subdirectory(lib/comms)
add_subdirectory(lib/storage) 
add_subdirectory(lib/storage/pico-vfs) 
add_subdirectory(lib/eventman)
add_subdirectory(lib/telemetry)

set(MAIN_SOURCES
    main.cpp
    lib/sensors/ISensor.cpp
    lib/eventman/event_manager.cpp
    lib/utils.cpp
    lib/storage/storage.cpp
)

set(GPS_SOURCES
    lib/location/gps_collector.cpp
)

add_executable(main
    ${MAIN_SOURCES}
    ${GPS_SOURCES}
)

target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -Wno-error
    -Wunused
    -Wunreachable-code
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/sensors
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/powerman
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/comms
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/comms/commands
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/location
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/location/NMEA
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/storage/pico-vfs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/storage/pico-vfs/vendor/littlefs
)

pico_enable_filesystem(${CMAKE_PROJECT_NAME} FS_INIT lib/storage/storage.cpp)
pico_enable_stdio_uart(main 1)
pico_enable_stdio_usb(main 0)

target_link_libraries(main PRIVATE
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_uart
    pico_multicore
    blockdevice_flash
    blockdevice_heap
    blockdevice_loopback
    blockdevice_sd
    filesystem_fat
    filesystem_littlefs
    filesystem_vfs
    Sensors_lib
    Comms_lib
    PowerManager_lib
    storage_lib
    clock_lib
    LoRa_pico_lib
    eventman_lib
    telemetry_lib
)

# Build information
file(READ "${CMAKE_SOURCE_DIR}/cmake/CMakeBuildNumberCache.txt" CURRENT_BUILD_NUMBER)
string(STRIP "${CURRENT_BUILD_NUMBER}" CURRENT_BUILD_NUMBER)
pico_set_program_name(main "kubisat_firmware")
pico_set_program_version(main "0.276")
pico_add_extra_outputs(main)

add_custom_command(
    TARGET main
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DLANGUAGE:STRING="C"
        -DHEADER_DIR:PATH="${CMAKE_SOURCE_DIR}"
        -DCACHE_DIR:PATH="${CMAKE_SOURCE_DIR}/cmake"
        -P "${CMAKE_SOURCE_DIR}/cmake/build_number.cmake"
)


add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/release"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2"
        "${CMAKE_CURRENT_SOURCE_DIR}/release/${PROJECT_NAME}_v${CURRENT_BUILD_NUMBER}.uf2"
    COMMENT "Copying ${PROJECT_NAME}.uf2 to release directory with build number"
    VERBATIM
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/documentation.cmake)

