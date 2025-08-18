/**
 * @file BME280.cpp
 * @brief Implementation of the BME280 environmental sensor class.
 *
 * This file contains the implementation of the BME280 class, which provides an
 * interface to the BME280 temperature, pressure, and humidity sensor using the
 * I2C communication protocol.
 */

#include "BME280.h"
#include <iomanip>
#include <vector>
#include <algorithm>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "utils.h"

/**
 * @brief Constructor for the BME280 class.
 * @param i2cPort Pointer to the I2C interface.
 * @param address I2C address of the BME280 sensor (default: ADDR_SDO_LOW).
 */
BME280::BME280(i2c_inst_t* i2cPort, uint8_t address)
    : i2c_port(i2cPort), device_addr(address), calib_params{}, initialized_(false), t_fine(0) {
}

/**
 * @brief Initializes the BME280 sensor.
 * @return True if initialization was successful, false otherwise.
 */
bool BME280::init() {
    if (!i2c_port) {
        uart_print("BME280 I2C port not initialized.", VerbosityLevel::ERROR);
        return false;
    }

    // Check device ID to confirm it's a BME280
    uint8_t chip_id;
    if (!read_register(0xD0, &chip_id)) {
        uart_print("Failed to read chip ID from BME280.", VerbosityLevel::ERROR);
        return false;
    }

    if (chip_id != 0x60) {
        uart_print("Invalid BME280 chip ID.", VerbosityLevel::ERROR);
        return false;
    }

    // Configure sensor
    if (!configure_sensor()) {
        uart_print("Failed to configure BME280 sensor.", VerbosityLevel::ERROR);
        return false;
    }

    // Retrieve calibration parameters
    if (!get_calibration_parameters()) {
        uart_print("Failed to get calibration parameters from BME280.", VerbosityLevel::ERROR);
        return false;
    }

    initialized_ = true;
    uart_print("BME280 initialized.", VerbosityLevel::INFO);
    return true;
}

/**
 * @brief Resets the BME280 sensor.
 */
void BME280::reset() {
    write_register(REG_RESET, 0xB6);
    sleep_ms(10); // Wait for reset to complete
}

/**
 * @brief Reads all raw data from the sensor.
 * @param temperature Pointer to store the raw temperature value.
 * @param pressure Pointer to store the raw pressure value.
 * @param humidity Pointer to store the raw humidity value.
 * @return True if the data was read successfully, false otherwise.
 */
bool BME280::read_raw_all(int32_t* temperature, int32_t* pressure, int32_t* humidity) {
    if (!initialized_) {
        uart_print("BME280 not initialized.", VerbosityLevel::ERROR);
        return false;
    }

    // Define the starting register address
    uint8_t start_reg = REG_PRESSURE_MSB;
    // Total bytes to read: 3 (pressure) + 3 (temperature) + 2 (humidity) = 8
    uint8_t buf[8] = {0};

    // Write the starting register address
    if (!write_register(start_reg, 1)) {
        uart_print("Failed to write to BME280.", VerbosityLevel::ERROR);
        return false;
    }

    // Read data
    int ret = i2c_read_blocking(i2c_port, device_addr, buf, 8, false);
    if (ret != 8) {
        uart_print("Failed to read from BME280.", VerbosityLevel::ERROR);
        return false;
    }

    // Combine bytes to form raw values
    *pressure = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)(buf[2] >> 4));
    *temperature = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)(buf[5] >> 4));
    *humidity = ((int32_t)buf[6] << 8) | (int32_t)buf[7];

    return true;
}

/**
 * @brief Converts raw temperature data to degrees Celsius.
 * @param temp_raw Raw temperature value.
 * @return Temperature in degrees Celsius.
 */
float BME280::convert_temperature(int32_t temp_raw) const {
    int32_t var1, var2;
    var1 = ((((temp_raw >> 3) - ((int32_t)calib_params.dig_t1 << 1))) * ((int32_t)calib_params.dig_t2)) >> 11;
    var2 = (((((temp_raw >> 4) - ((int32_t)calib_params.dig_t1)) * ((temp_raw >> 4) - ((int32_t)calib_params.dig_t1))) >> 12) * ((int32_t)calib_params.dig_t3)) >> 14;
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0f;
}

/**
 * @brief Converts raw pressure data to hectopascals (hPa).
 * @param pressure_raw Raw pressure value.
 * @return Pressure in hPa.
 */
float BME280::convert_pressure(int32_t pressure_raw) const {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_params.dig_p6;
    var2 = var2 + ((var1 * (int64_t)calib_params.dig_p5) << 17);
    var2 = var2 + (((int64_t)calib_params.dig_p4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_params.dig_p3) >> 8) + ((var1 * (int64_t)calib_params.dig_p2) << 12);
    var1 = ((((int64_t)1 << 47) + var1)) * ((int64_t)calib_params.dig_p1) >> 33;

    if (var1 == 0) {
        return 0.0f; // avoid exception caused by division by zero
    }
    p = 1048576 - pressure_raw;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_params.dig_p9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_params.dig_p8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_params.dig_p7) << 4);
    return (float)p / 25600.0f; // in hPa
}

/**
 * @brief Converts raw humidity data to relative humidity (%).
 * @param humidity_raw Raw humidity value.
 * @return Relative humidity in %.
 */
float BME280::convert_humidity(int32_t humidity_raw) const {
    int32_t v_x1_u32r;
    v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((humidity_raw << 14) - ((int32_t)calib_params.dig_h4 << 20) - ((int32_t)calib_params.dig_h5 * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * (int32_t)calib_params.dig_h6) >> 10) * (((v_x1_u32r * (int32_t)calib_params.dig_h3) >> 11) + 32768)) >> 10) + 2097152) *
                   (int32_t)calib_params.dig_h2 + 8192) >> 14));
    v_x1_u32r = std::max(v_x1_u32r, (int32_t)0);
    v_x1_u32r = std::min(v_x1_u32r, (int32_t)419430400);
    float h = v_x1_u32r >> 12;
    return h / 1024.0f;
}

/**
 * @brief Retrieves the calibration parameters from the sensor.
 * @return True if the parameters were read successfully, false otherwise.
 */
bool BME280::get_calibration_parameters() {
    // Read temperature and pressure calibration data (0x88 to 0xA1)
    uint8_t calib_data[NUM_CALIB_PARAMS];
    if (!read_register(REG_DIG_T1_LSB, calib_data, NUM_CALIB_PARAMS)) {
        uart_print("Failed to read calibration data from BME280.", VerbosityLevel::ERROR);
        return false;
    }

    // Parse temperature calibration data
    calib_params.dig_t1 = (uint16_t)(calib_data[1] << 8 | calib_data[0]);
    calib_params.dig_t2 = (int16_t)(calib_data[3] << 8 | calib_data[2]);
    calib_params.dig_t3 = (int16_t)(calib_data[5] << 8 | calib_data[4]);

    // Parse pressure calibration data
    calib_params.dig_p1 = (uint16_t)(calib_data[7] << 8 | calib_data[6]);
    calib_params.dig_p2 = (int16_t)(calib_data[9] << 8 | calib_data[8]);
    calib_params.dig_p3 = (int16_t)(calib_data[11] << 8 | calib_data[10]);
    calib_params.dig_p4 = (int16_t)(calib_data[13] << 8 | calib_data[12]);
    calib_params.dig_p5 = (int16_t)(calib_data[15] << 8 | calib_data[14]);
    calib_params.dig_p6 = (int16_t)(calib_data[17] << 8 | calib_data[16]);
    calib_params.dig_p7 = (int16_t)(calib_data[19] << 8 | calib_data[18]);
    calib_params.dig_p8 = (int16_t)(calib_data[21] << 8 | calib_data[20]);
    calib_params.dig_p9 = (int16_t)(calib_data[23] << 8 | calib_data[22]);

    calib_params.dig_h1 = calib_data[25];

    // Read humidity calibration data (0xE1 to 0xE7)
    uint8_t hum_calib_data[NUM_HUM_CALIB_PARAMS];
    if (!read_register(REG_DIG_H2, hum_calib_data, NUM_HUM_CALIB_PARAMS)) {
        uart_print("Failed to read humidity calibration data from BME280.", VerbosityLevel::ERROR);
        return false;
    }

    // Parse humidity calibration data
    calib_params.dig_h2 = (int16_t)(hum_calib_data[1] << 8 | hum_calib_data[0]);
    calib_params.dig_h3 = hum_calib_data[2];
    calib_params.dig_h4 = (int16_t)((hum_calib_data[3] << 4) | (hum_calib_data[4] & 0x0F));
    calib_params.dig_h5 = (int16_t)((hum_calib_data[5] << 4) | (hum_calib_data[4] >> 4));
    calib_params.dig_h6 = (int8_t)hum_calib_data[6];

    return true;
}

/**
 * @brief Configures the sensor with default settings.
 * @return True if the configuration was successful, false otherwise.
 */
bool BME280::configure_sensor() {
    // Set humidity oversampling (must be set before ctrl_meas)
    if (!write_register(REG_CTRL_HUM, HUMIDITY_OVERSAMPLING)) {
        uart_print("Failed to write CTRL_HUM to BME280.", VerbosityLevel::ERROR);
        return false;
    }

    // Write config register
    if (!write_register(REG_CONFIG, 0x00)) {
        uart_print("Failed to write CONFIG to BME280.", VerbosityLevel::ERROR);
        return false;
    }

    // Write ctrl_meas register
    if (!write_register(REG_CTRL_MEAS, NORMAL_MODE)) {
        uart_print("Failed to write CTRL_MEAS to BME280.", VerbosityLevel::ERROR);
        return false;
    }

    return true;
}

/**
 * @brief Helper function for I2C writes.
 * @param reg Register address to write to.
 * @param value Value to write to the register.
 * @return True if the write was successful, false otherwise.
 */
bool BME280::write_register(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    int ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    return (ret == 2);
}

/**
 * @brief Helper function for I2C reads with a specified length.
 * @param reg Register address to read from.
 * @param data Pointer to store the read data.
 * @param len Number of bytes to read.
 * @return True if the read was successful, false otherwise.
 */
bool BME280::read_register(uint8_t reg, uint8_t* data, size_t len) {
    int ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        return false;
    }
    ret = i2c_read_blocking(i2c_port, device_addr, data, len, false);
    return (static_cast<size_t>(ret) == len);
}

/**
 * @brief Helper function for I2C reads.
 * @param reg Register address to read from.
 * @param data Pointer to store the read data.
 * @return True if the read was successful, false otherwise.
 */
bool BME280::read_register(uint8_t reg, uint8_t* data) {
    return read_register(reg, data, 1);
}