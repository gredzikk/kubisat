# lib/CMakeLists.txt

cmake_minimum_required(VERSION 3.13)

# Define the library
add_library(BH1750 STATIC BH1750.cpp)

# Specify include directories for the library
target_include_directories(BH1750 PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Link necessary libraries
target_link_libraries(BH1750 pico_stdlib hardware_i2c)