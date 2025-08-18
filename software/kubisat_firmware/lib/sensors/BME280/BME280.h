/**
 * @file BME280.h
* @brief Header file for the BME280 environmental sensor class.
*
* This class provides an interface to the BME280 temperature, pressure, and humidity sensor
* using the I2C communication protocol. It includes functions for initialization, reading raw
* sensor data, converting raw data to physical units, and configuring the sensor's operating mode.
*/

#ifndef BME280_H
#define BME280_H

#include <cstdint>
#include <iostream>
#include "hardware/i2c.h"

/**
 * @brief Structure to hold the BME280 calibration parameters.
 *
 * These parameters are unique to each sensor and are used to compensate for
 * manufacturing variations and improve the accuracy of the sensor readings.
 */
struct BME280CalibParam {
    /** @brief Temperature calibration parameter 1 */
    uint16_t dig_t1;
    /** @brief Temperature calibration parameter 2 */
    int16_t  dig_t2;
    /** @brief Temperature calibration parameter 3 */
    int16_t  dig_t3;

    /** @brief Pressure calibration parameter 1 */
    uint16_t dig_p1;
    /** @brief Pressure calibration parameter 2 */
    int16_t  dig_p2;
    /** @brief Pressure calibration parameter 3 */
    int16_t  dig_p3;
    /** @brief Pressure calibration parameter 4 */
    int16_t  dig_p4;
    /** @brief Pressure calibration parameter 5 */
    int16_t  dig_p5;
    /** @brief Pressure calibration parameter 6 */
    int16_t  dig_p6;
    /** @brief Pressure calibration parameter 7 */
    int16_t  dig_p7;
    /** @brief Pressure calibration parameter 8 */
    int16_t  dig_p8;
    /** @brief Pressure calibration parameter 9 */
    int16_t  dig_p9;

    /** @brief Humidity calibration parameter 1 */
    uint8_t  dig_h1;
    /** @brief Humidity calibration parameter 2 */
    int16_t  dig_h2;
    /** @brief Humidity calibration parameter 3 */
    uint8_t  dig_h3;
    /** @brief Humidity calibration parameter 4 */
    int16_t  dig_h4;
    /** @brief Humidity calibration parameter 5 */
    int16_t  dig_h5;
    /** @brief Humidity calibration parameter 6 */
    int8_t   dig_h6;
};

/**
 * @brief Class to interface with the BME280 environmental sensor.
 *
 * This class provides methods to initialize the sensor, read raw data,
 * convert raw data to physical units (temperature, pressure, humidity),
 * and configure the sensor's operating mode.
 */
class BME280 {
public:
    /**
     * @brief I2C Address Options for the BME280 sensor.
     */
    enum {
        /** @brief I2C address when SDO pin is low */
        ADDR_SDO_LOW = 0x76,
        /** @brief I2C address when SDO pin is high */
        ADDR_SDO_HIGH = 0x77
    };

    /**
     * @brief Enum class for oversampling settings.
     *
     * These settings determine the number of measurements that are averaged
     * to reduce noise and improve the accuracy of the sensor readings.
     */
    enum class Oversampling : uint8_t {
        /** @brief No oversampling */
        OSR_X0 = 0x00,
        /** @brief 1x oversampling */
        OSR_X1 = 0x01,
        /** @brief 2x oversampling */
        OSR_X2 = 0x02,
        /** @brief 4x oversampling */
        OSR_X4 = 0x03,
        /** @brief 8x oversampling */
        OSR_X8 = 0x04,
        /** @brief 16x oversampling */
        OSR_X16 = 0x05
    };

    /**
     * @brief Constructor for the BME280 class.
     * @param i2cPort Pointer to the I2C interface.
     * @param address I2C address of the BME280 sensor (default: ADDR_SDO_LOW).
     */
    BME280(i2c_inst_t* i2cPort, uint8_t address = ADDR_SDO_LOW);

    /**
     * @brief Initializes the BME280 sensor.
     * @return True if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * @brief Resets the BME280 sensor.
     */
    void reset();

    /**
     * @brief Reads all raw data from the sensor.
     * @param temperature Pointer to store the raw temperature value.
     * @param pressure Pointer to store the raw pressure value.
     * @param humidity Pointer to store the raw humidity value.
     * @return True if the data was read successfully, false otherwise.
     */
    bool read_raw_all(int32_t* temperature, int32_t* pressure, int32_t* humidity);

    /**
     * @brief Converts raw temperature data to degrees Celsius.
     * @param temp_raw Raw temperature value.
     * @return Temperature in degrees Celsius.
     */
    float convert_temperature(int32_t temp_raw) const;

    /**
     * @brief Converts raw pressure data to hectopascals (hPa).
     * @param pressure_raw Raw pressure value.
     * @return Pressure in hPa.
     */
    float convert_pressure(int32_t pressure_raw) const;

    /**
     * @brief Converts raw humidity data to relative humidity (%).
     * @param humidity_raw Raw humidity value.
     * @return Relative humidity in %.
     */
    float convert_humidity(int32_t humidity_raw) const;

private:
    /**
     * @brief Helper function for I2C writes.
     * @param reg Register address to write to.
     * @param value Value to write to the register.
     * @return True if the write was successful, false otherwise.
     */
    bool write_register(uint8_t reg, uint8_t value);

    /**
     * @brief Helper function for I2C reads.
     * @param reg Register address to read from.
     * @param data Pointer to store the read data.
     * @return True if the read was successful, false otherwise.
     */
    bool read_register(uint8_t reg, uint8_t* data);

    /**
     * @brief Helper function for I2C reads with a specified length.
     * @param reg Register address to read from.
     * @param data Pointer to store the read data.
     * @param len Number of bytes to read.
     * @return True if the read was successful, false otherwise.
     */
    bool read_register(uint8_t reg, uint8_t* data, size_t len);

    /**
     * @brief Configures the sensor with default settings.
     * @return True if the configuration was successful, false otherwise.
     */
    bool configure_sensor();

    /**
     * @brief Retrieves the calibration parameters from the sensor.
     * @return True if the parameters were read successfully, false otherwise.
     */
    bool get_calibration_parameters();

    /** @brief Pointer to the I2C interface */
    i2c_inst_t* i2c_port;
    /** @brief I2C device address */
    uint8_t device_addr;

    /** @brief Calibration parameters for the sensor */
    BME280CalibParam calib_params;

    /** @brief Initialization status of the sensor */
    bool initialized_;

    /** @brief Fine temperature parameter needed for compensation */
    mutable int32_t t_fine;

    /**
     * @brief Register Definitions for the BME280 sensor.
     */
    enum {
        REG_CONFIG            = 0xF5,  ///< Configuration register
        REG_CTRL_MEAS         = 0xF4,  ///< Control measurement register
        REG_CTRL_HUM          = 0xF2,  ///< Control humidity register
        REG_RESET             = 0xE0,  ///< Reset register

        REG_PRESSURE_MSB      = 0xF7,  ///< Pressure data MSB
        REG_TEMPERATURE_MSB   = 0xFA,  ///< Temperature data MSB
        REG_HUMIDITY_MSB      = 0xFD,  ///< Humidity data MSB

        // Calibration Registers
        REG_DIG_T1_LSB        = 0x88,  ///< Calibration data LSB
        REG_DIG_T1_MSB        = 0x89,  ///< Calibration data MSB
        REG_DIG_T2_LSB        = 0x8A,  ///< Calibration data LSB
        REG_DIG_T2_MSB        = 0x8B,  ///< Calibration data MSB
        REG_DIG_T3_LSB        = 0x8C,  ///< Calibration data LSB
        REG_DIG_T3_MSB        = 0x8D,  ///< Calibration data MSB

        REG_DIG_P1_LSB        = 0x8E,  ///< Calibration data LSB
        REG_DIG_P1_MSB        = 0x8F,  ///< Calibration data MSB
        REG_DIG_P2_LSB        = 0x90,  ///< Calibration data LSB
        REG_DIG_P2_MSB        = 0x91,  ///< Calibration data MSB
        REG_DIG_P3_LSB        = 0x92,  ///< Calibration data LSB
        REG_DIG_P3_MSB        = 0x93,  ///< Calibration data MSB
        REG_DIG_P4_LSB        = 0x94,  ///< Calibration data LSB
        REG_DIG_P4_MSB        = 0x95,  ///< Calibration data MSB
        REG_DIG_P5_LSB        = 0x96,  ///< Calibration data LSB
        REG_DIG_P5_MSB        = 0x97,  ///< Calibration data MSB
        REG_DIG_P6_LSB        = 0x98,  ///< Calibration data LSB
        REG_DIG_P6_MSB        = 0x99,  ///< Calibration data MSB
        REG_DIG_P7_LSB        = 0x9A,  ///< Calibration data LSB
        REG_DIG_P7_MSB        = 0x9B,  ///< Calibration data MSB
        REG_DIG_P8_LSB        = 0x9C,  ///< Calibration data LSB
        REG_DIG_P8_MSB        = 0x9D,  ///< Calibration data MSB
        REG_DIG_P9_LSB        = 0x9E,  ///< Calibration data LSB
        REG_DIG_P9_MSB        = 0x9F,  ///< Calibration data MSB

        // Humidity Calibration Registers
        REG_DIG_H1            = 0xA1,  ///< Humidity calibration data
        REG_DIG_H2            = 0xE1,  ///< Humidity calibration data
        REG_DIG_H3            = 0xE3,  ///< Humidity calibration data
        REG_DIG_H4            = 0xE4,  ///< Humidity calibration data
        REG_DIG_H5            = 0xE5,  ///< Humidity calibration data
        REG_DIG_H6            = 0xE7   ///< Humidity calibration data
    };

    /**
     * @brief Sensor settings.
     */
    enum {
        HUMIDITY_OVERSAMPLING = static_cast<uint8_t>(Oversampling::OSR_X16),  ///< Humidity oversampling setting
        TEMPERATURE_OVERSAMPLING = static_cast<uint8_t>(Oversampling::OSR_X16), ///< Temperature oversampling setting
        PRESSURE_OVERSAMPLING = static_cast<uint8_t>(Oversampling::OSR_X16),    ///< Pressure oversampling setting
        NORMAL_MODE = 0xB7                                                        ///< Normal mode setting
    };

    /**
     * @brief Calibration data length.
     */
    enum {
        NUM_CALIB_PARAMS = 26,  ///< Number of calibration parameters
        NUM_HUM_CALIB_PARAMS = 7   ///< Number of humidity calibration parameters
    };
};

 #endif // BME280_H