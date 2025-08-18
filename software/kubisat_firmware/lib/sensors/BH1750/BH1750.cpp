/**
 * @file BH1750.cpp
 * @brief Implementation of the BH1750 light sensor class.
 *
 * This file contains the implementation of the BH1750 class, which provides an
 * interface to the BH1750 digital light sensor using the I2C communication protocol.
 */

#include "BH1750.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <iostream>

/**
 * @ingroup BH1750
 * @brief Constructor for the BH1750 class.
 * @param i2c Pointer to the I2C interface.
 * @param addr I2C address of the BH1750 sensor (default: 0x23).
 */
BH1750::BH1750(i2c_inst_t* i2c, uint8_t addr) : _i2c_addr(addr), i2c_port_(i2c) {}

/**
 * @ingroup BH1750
 * @brief Initializes the BH1750 sensor.
 * @param mode Measurement mode to use (default: CONTINUOUS_HIGH_RES_MODE).
 * @return True if initialization was successful, false otherwise.
 */
bool BH1750::begin(Mode mode) {
    write8(static_cast<uint8_t>(Mode::POWER_ON));
    write8(static_cast<uint8_t>(Mode::RESET));
    bool config_status = configure(mode);

    return config_status;
}

/**
 * @ingroup BH1750
 * @brief Configures the BH1750 sensor with the specified mode.
 * @param mode Measurement mode to configure.
 * @return True if configuration was successful, false otherwise.
 */
bool BH1750::configure(Mode mode) {
    uint8_t modeVal = static_cast<uint8_t>(mode);
    switch (mode) {
        case Mode::UNCONFIGURED_POWER_DOWN:
        case Mode::POWER_ON:
        case Mode::RESET:
        case Mode::CONTINUOUS_HIGH_RES_MODE:
        case Mode::CONTINUOUS_HIGH_RES_MODE_2:
        case Mode::CONTINUOUS_LOW_RES_MODE:
        case Mode::ONE_TIME_HIGH_RES_MODE:
        case Mode::ONE_TIME_HIGH_RES_MODE_2:
        case Mode::ONE_TIME_LOW_RES_MODE:
            write8(modeVal);
            sleep_ms(10);
            return true;
        default:
            return false;
    }
}

/**
 * @ingroup BH1750
 * @brief Reads the light level from the BH1750 sensor.
 * @return Light level in lux.
 */
float BH1750::get_light_level() {
    uint8_t buffer[2];
    i2c_read_blocking(i2c_port_, _i2c_addr, buffer, 2, false);
    uint16_t level = (buffer[0] << 8) | buffer[1];

    float lux = static_cast<float>(level) / 1.2f;
    return lux;
}

/**
 * @ingroup BH1750
 * @brief Writes a single byte of data to the BH1750 sensor.
 * @param data Byte of data to write.
 */
void BH1750::write8(uint8_t data) {
    uint8_t buf[1] = {data};
    i2c_write_blocking(i2c_port_, _i2c_addr, buf, 1, false);
}