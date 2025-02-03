# CMakeLists.txt

cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
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

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(bh1750_test C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add the BH1750 library
add_subdirectory(lib)

# Add executable. Default name is the project name, version 0.1
add_executable(bh1750_test bh1750_test.cpp)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(bh1750_test 0)
pico_enable_stdio_usb(bh1750_test 1)

# Link the BH1750 library to the executable
target_link_libraries(bh1750_test
    pico_stdlib
    hardware_i2c
    BH1750
)

# Add the standard include files to the build
target_include_directories(bh1750_test PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(bh1750_test)