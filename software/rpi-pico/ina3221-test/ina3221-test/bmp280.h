#ifndef BMP280_H
#define BMP280_H

#include <cstdint>
#include <iostream>
#include "hardware/i2c.h"

// Calibration parameters structure
struct BMP280CalibParam {
    // Temperature parameters
    uint16_t dig_t1;
    int16_t  dig_t2;
    int16_t  dig_t3;

    // Pressure parameters
    uint16_t dig_p1;
    int16_t  dig_p2;
    int16_t  dig_p3;
    int16_t  dig_p4;
    int16_t  dig_p5;
    int16_t  dig_p6;
    int16_t  dig_p7;
    int16_t  dig_p8;
    int16_t  dig_p9;
};

// BMP280 Class Definition
class BMP280 {
public:
    // I2C Address Options
    static constexpr uint8_t ADDR_SDO_LOW = 0x76;
    static constexpr uint8_t ADDR_SDO_HIGH = 0x77;

    // Constructor
    BMP280(i2c_inst_t* i2cPort, uint8_t address = ADDR_SDO_LOW);

    // Initialize the sensor
    bool init();

    // Reset the sensor
    void reset();

    // Read raw temperature and pressure data
    bool read_raw(int32_t* temperature, int32_t* pressure);

    // Convert raw data to actual temperature in Celsius
    float convert_temperature(int32_t temp_raw) const;

    // Convert raw data to actual pressure in kPa
    float convert_pressure(int32_t pressure_raw, int32_t temp_raw) const;

private:
    // Configure the sensor
    bool configure_sensor();

    // Retrieve calibration parameters from the sensor
    bool get_calibration_parameters();

    // I2C port and device address
    i2c_inst_t* i2c_port;
    uint8_t device_addr;

    // Calibration parameters
    BMP280CalibParam calib_params;

    // Initialization status
    bool initialized;

    // Fine temperature parameter needed for pressure compensation
    mutable int32_t t_fine;

    // Register Definitions
    static constexpr uint8_t REG_CONFIG        = 0xF5;
    static constexpr uint8_t REG_CTRL_MEAS     = 0xF4;
    static constexpr uint8_t REG_RESET         = 0xE0;

    static constexpr uint8_t REG_PRESSURE_MSB  = 0xF7;

    // Calibration Registers
    static constexpr uint8_t REG_DIG_T1_LSB    = 0x88;
    static constexpr uint8_t REG_DIG_T1_MSB    = 0x89;
    static constexpr uint8_t REG_DIG_T2_LSB    = 0x8A;
    static constexpr uint8_t REG_DIG_T2_MSB    = 0x8B;
    static constexpr uint8_t REG_DIG_T3_LSB    = 0x8C;
    static constexpr uint8_t REG_DIG_T3_MSB    = 0x8D;
    static constexpr uint8_t REG_DIG_P1_LSB    = 0x8E;
    static constexpr uint8_t REG_DIG_P1_MSB    = 0x8F;
    static constexpr uint8_t REG_DIG_P2_LSB    = 0x90;
    static constexpr uint8_t REG_DIG_P2_MSB    = 0x91;
    static constexpr uint8_t REG_DIG_P3_LSB    = 0x92;
    static constexpr uint8_t REG_DIG_P3_MSB    = 0x93;
    static constexpr uint8_t REG_DIG_P4_LSB    = 0x94;
    static constexpr uint8_t REG_DIG_P4_MSB    = 0x95;
    static constexpr uint8_t REG_DIG_P5_LSB    = 0x96;
    static constexpr uint8_t REG_DIG_P5_MSB    = 0x97;
    static constexpr uint8_t REG_DIG_P6_LSB    = 0x98;
    static constexpr uint8_t REG_DIG_P6_MSB    = 0x99;
    static constexpr uint8_t REG_DIG_P7_LSB    = 0x9A;
    static constexpr uint8_t REG_DIG_P7_MSB    = 0x9B;
    static constexpr uint8_t REG_DIG_P8_LSB    = 0x9C;
    static constexpr uint8_t REG_DIG_P8_MSB    = 0x9D;
    static constexpr uint8_t REG_DIG_P9_LSB    = 0x9E;
    static constexpr uint8_t REG_DIG_P9_MSB    = 0x9F;

    // Number of calibration parameters to read
    static constexpr size_t NUM_CALIB_PARAMS = 24;
};

#endif // BMP280_H