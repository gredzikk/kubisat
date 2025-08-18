/**
 * @file BH1750.h
 * @brief Header file for the BH1750 light sensor class.
 *
 * This class provides an interface to the BH1750 digital light sensor
 * using the I2C communication protocol.
 */

#ifndef __BH1750_H__
#define __BH1750_H__

#include "hardware/i2c.h"

/**
 * @defgroup BH1750 BH1750 Light Sensor
 * @ingroup sensors
 * @brief Driver for the BH1750 digital light sensor.
 */

/**
 * @defgroup BH1750_Constants Constants
 * @ingroup BH1750
 * @brief Defines constants used by the BH1750 driver.
 */

/**
 * @defgroup BH1750_Types Types
 * @ingroup BH1750
 * @brief Defines types used by the BH1750 driver.
 */

/**
 * @ingroup BH1750_Constants
 * @brief Correct content of WHO_AM_I register (not actually used in this driver).
 */
#define _BH1750_DEVICE_ID 0xE1

/**
 * @ingroup BH1750_Constants
 * @brief Minimum value for the MTREG register.
 */
#define _BH1750_MTREG_MIN 31

/**
 * @ingroup BH1750_Constants
 * @brief Maximum value for the MTREG register.
 */
#define _BH1750_MTREG_MAX 254

/**
 * @ingroup BH1750_Constants
 * @brief Default value for the MTREG register.
 */
#define _BH1750_DEFAULT_MTREG 69

/**
 * @ingroup BH1750
 * @brief Class to interface with the BH1750 light sensor.
 */
class BH1750 {
public:
    /**
     * @ingroup BH1750_Types
     * @brief Enumeration of measurement modes for the BH1750 sensor.
     */
    enum class Mode : uint8_t {
        /** @brief Power down mode */
        UNCONFIGURED_POWER_DOWN = 0x00,
        /** @brief Power on mode */
        POWER_ON = 0x01,
        /** @brief Reset mode */
        RESET = 0x07,
        /** @brief Continuous high resolution mode */
        CONTINUOUS_HIGH_RES_MODE = 0x10,
        /** @brief Continuous high resolution mode 2 */
        CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
        /** @brief Continuous low resolution mode */
        CONTINUOUS_LOW_RES_MODE = 0x13,
        /** @brief One-time high resolution mode */
        ONE_TIME_HIGH_RES_MODE = 0x20,
        /** @brief One-time high resolution mode 2 */
        ONE_TIME_HIGH_RES_MODE_2 = 0x21,
        /** @brief One-time low resolution mode */
        ONE_TIME_LOW_RES_MODE = 0x23
    };

    /**
     * @brief Constructor for the BH1750 class.
     * @param i2c Pointer to the I2C interface.
     * @param addr I2C address of the BH1750 sensor (default: 0x23).
     */
    BH1750(i2c_inst_t* i2c, uint8_t addr = 0x23);

    /**
     * @brief Initializes the BH1750 sensor.
     * @param mode Measurement mode to use (default: CONTINUOUS_HIGH_RES_MODE).
     * @return True if initialization was successful, false otherwise.
     */
    bool begin(Mode mode = Mode::CONTINUOUS_HIGH_RES_MODE);

    /**
     * @brief Configures the BH1750 sensor with the specified mode.
     * @param mode Measurement mode to configure.
     * @return True if configuration was successful, false otherwise.
     */
    bool configure(Mode mode);

    /**
     * @brief Reads the light level from the BH1750 sensor.
     * @return Light level in lux.
     */
    float get_light_level();

private:
    /**
     * @brief Writes a single byte of data to the BH1750 sensor.
     * @param data Byte of data to write.
     */
    void write8(uint8_t data);

    /** @brief I2C address of the BH1750 sensor */
    uint8_t _i2c_addr;
    /** @brief Pointer to the I2C interface */
    i2c_inst_t* i2c_port_;
};

 #endif // __BH1750_H__