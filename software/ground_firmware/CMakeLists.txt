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

set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/documentation.cmake)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_LORA_PATH ${PROJECT_SOURCE_DIR})

set(PICO_CXX_ENABLE_EXCEPTIONS 1)
pico_sdk_init()

add_subdirectory(LoRa) 
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
    add_executable(main
        main.cpp
        pin_config.cpp
        utils.cpp
    )
    
    target_link_libraries(main PRIVATE
        pico_stdlib
        LoRa_pico_lib
    )
    
    pico_enable_stdio_usb(main 0)
    pico_enable_stdio_uart(main 1)
    
    pico_add_extra_outputs(main)

    add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/release"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.uf2"
        "${CMAKE_CURRENT_SOURCE_DIR}/release/${PROJECT_NAME}.uf2"
    COMMENT "Copying ${PROJECT_NAME}.uf2 to release directory"
)
endif()