# lib/Sensors/CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# Add subdirectories for each sensor
add_subdirectory(BH1750)
add_subdirectory(BME280)


# Create a sensor library that includes all sensor libraries
add_library(Sensors_lib INTERFACE)

# Include directories for the sensor library
target_include_directories(Sensors_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/BH1750
    ${CMAKE_CURRENT_SOURCE_DIR}/BME280
)

# Link the sensor libraries to the sensor library
target_link_libraries(Sensors_lib INTERFACE
    BH1750_lib
    BME280_lib
)