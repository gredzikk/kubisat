add_library(MPU6050_lib STATIC
    MPU6050.cpp
    MPU6050.h
    MPU6050_WRAPPER.cpp
    MPU6050_WRAPPER.h
)
target_include_directories(MPU6050_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib/sensors
    ${CMAKE_SOURCE_DIR}/lib
)
target_link_libraries(MPU6050_lib PUBLIC
    pico_stdlib
    hardware_i2c
)
