# lib/Sensors/CMakeLists.txt

# Add subdirectories for each sensor
add_subdirectory(BH1750)
add_subdirectory(BME280)
add_subdirectory(MPU6050)

add_library(Sensors_lib INTERFACE)

target_include_directories(Sensors_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/BH1750
    ${CMAKE_CURRENT_SOURCE_DIR}/BME280
    ${CMAKE_CURRENT_SOURCE_DIR}/MPU6050
)

target_link_libraries(Sensors_lib INTERFACE
    BH1750_lib
    BME280_lib
    MPU6050_lib
)