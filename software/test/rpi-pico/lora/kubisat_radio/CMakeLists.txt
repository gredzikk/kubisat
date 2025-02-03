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

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)


project(LoRa-RP2040 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_LORA_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
pico_sdk_init()

# Add the library subdirectory
add_subdirectory(LoRa) # Assuming library is in src directory

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
)

if (TARGET tinyusb_device)
    # First create the executable target
    add_executable(main
        main.cpp
        commands.cpp
    )
    
    target_link_libraries(main PRIVATE
        pico_stdlib
        LoRa_pico_lib
    )
    
    pico_enable_stdio_usb(main 1)
    pico_enable_stdio_uart(main 0)
    
    pico_add_extra_outputs(main)
endif()