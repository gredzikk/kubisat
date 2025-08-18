// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file ISensor.h
 * @brief Header file for the ISensor interface and SensorWrapper class.
 *
 * @details This file defines the ISensor interface, which provides a common
 *          interface for interacting with different types of sensors. It also
 *          defines the SensorWrapper class, which manages a collection of
 *          sensors and provides methods for initializing, configuring, and
 *          reading data from them.
 *
 * @defgroup Sensors Sensors
 * @brief Classes for handling sensor-related functions.
 *
 * @{
 */

#ifndef ISENSOR_H
#define ISENSOR_H

#include <map>
#include <string>
#include <vector>
#include <utility>
#include "hardware/i2c.h"

/**
 * @brief Enumeration of sensor types.
 * @details Defines the different types of sensors that can be managed.
 * @ingroup Sensors
 */
enum class SensorType : uint8_t
{
    /** @brief No sensor. */
    NONE = 0x00,
    /** @brief Light sensor. */
    LIGHT = 0x01,
    /** @brief Environment sensor. */
    ENVIRONMENT = 0x02,
};

/**
 * @brief Enumeration of sensor data type identifiers.
 * @details Defines the different types of data that can be read from a sensor.
 * @ingroup Sensors
 */
enum class SensorDataTypeIdentifier : uint8_t
{
    /** @brief No data. */
    NONE = 0x00,
    /** @brief Light level. */
    LIGHT_LEVEL = 0x01,
    /** @brief Temperature. */
    TEMPERATURE = 0x02,
    /** @brief Humidity. */
    HUMIDITY = 0x03,
    /** @brief Pressure. */
    PRESSURE = 0x04,
};

/**
 * @brief Abstract base class for sensors.
 * @details Defines the interface for interacting with different types of sensors.
 * @ingroup Sensors
 */
class ISensor
{
public:
    /**
     * @brief Virtual destructor.
     * @details Ensures proper cleanup of derived classes.
     */
    virtual ~ISensor() = default;

    /**
     * @brief Initializes the sensor.
     * @return True if initialization was successful, false otherwise.
     */
    virtual bool init() = 0;

    /**
     * @brief Reads data from the sensor.
     * @param[in] type Data type to read.
     * @return The sensor data.
     */
    virtual float read_data(SensorDataTypeIdentifier type) = 0;

    /**
     * @brief Checks if the sensor is initialized.
     * @return True if the sensor is initialized, false otherwise.
     */
    virtual bool is_initialized() const = 0;

    /**
     * @brief Gets the sensor type.
     * @return The sensor type.
     */
    virtual SensorType get_type() const = 0;

    /**
     * @brief Configures the sensor.
     * @param[in] config A map of configuration parameters.
     * @return True if configuration was successful, false otherwise.
     */
    virtual bool configure(const std::map<std::string, std::string> &config) = 0;

    /**
     * @brief Gets the I2C address of the sensor.
     * @return The I2C address of the sensor.
     */
    virtual uint8_t get_address() const = 0;
};

/**
 * @brief Manages a collection of sensors.
 * @details This class provides methods for initializing, configuring, and
 *          reading data from different types of sensors.
 * @ingroup Sensors
 */
class SensorWrapper
{
public:
    /**
     * @brief Gets the singleton instance of the SensorWrapper class.
     * @return A reference to the singleton instance.
     */
    static SensorWrapper &get_instance()
    {
        static SensorWrapper instance;
        return instance;
    }

    /**
     * @brief Initializes a sensor.
     * @param[in] type Sensor type to initialize.
     * @param[in] i2c I2C instance to use for communication.
     * @return True if initialization was successful, false otherwise.
     */
    bool sensor_init(SensorType type, i2c_inst_t *i2c = nullptr);

    /**
     * @brief Configures a sensor.
     * @param[in] type Sensor type to configure.
     * @param[in] config A map of configuration parameters.
     * @return True if configuration was successful, false otherwise.
     */
    bool sensor_configure(SensorType type, const std::map<std::string, std::string> &config);

    /**
     * @brief Reads data from a sensor.
     * @param[in] sensorType Sensor type to read from.
     * @param[in] dataType Data type to read.
     * @return The sensor data.
     */
    float sensor_read_data(SensorType sensorType, SensorDataTypeIdentifier dataType);

    /**
     * @brief Gets a sensor.
     * @param[in] type Sensor type to get.
     * @return A pointer to the sensor.
     */
    ISensor *get_sensor(SensorType type);

    /**
     * @brief Scans for connected sensors.
     * @param[in] i2c I2C instance to use for scanning.
     * @return A vector of pairs, where each pair contains a sensor type and its address.
     */
    std::vector<std::pair<SensorType, uint8_t>> scan_connected_sensors(i2c_inst_t *i2c);

    /**
     * @brief Gets a list of available sensors.
     * @return A vector of pairs, where each pair contains a sensor type and its address.
     */
    std::vector<std::pair<SensorType, uint8_t>> get_available_sensors();

private:
    /** @brief Map of sensor types to sensor instances. */
    std::map<SensorType, ISensor *> sensors;

    /**
     * @brief Private constructor for the singleton pattern.
     */
    SensorWrapper() = default;
};

#endif
/** @} */