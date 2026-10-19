// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file ISensor.cpp
 * @brief Implementation of the ISensor interface and SensorWrapper class.
 *
 * @details This file implements the ISensor interface and SensorWrapper class,
 *          which provide a common interface for interacting with different
 *          types of sensors.
 *
 * @defgroup Sensors Sensors
 * @brief Classes for handling sensor-related functions.
 *
 * @{
 */

#include "ISensor.h"
#include "lib/sensors/BH1750/BH1750_WRAPPER.h"
#include "lib/sensors/BME280/BME280_WRAPPER.h"
#include "lib/sensors/MPU6050/MPU6050_WRAPPER.h"
#include "lib/utils.h"

/**
 * @brief Initializes a sensor.
 * @param[in] type Sensor type to initialize.
 * @param[in] i2c I2C instance to use for communication.
 * @return True if initialization was successful, false otherwise.
 * @ingroup Sensors
 */
bool SensorWrapper::sensor_init(SensorType type, i2c_inst_t *i2c)
{
    if (sensors.find(type) == sensors.end() || sensors[type] == nullptr)
    {
        switch (type)
        {
        case SensorType::LIGHT:
            sensors[type] = new BH1750Wrapper(i2c);
            break;
        case SensorType::ENVIRONMENT:
            sensors[type] = new BME280Wrapper(i2c);
            break;
        case SensorType::IMU:
            sensors[type] = new MPU6050Wrapper(i2c);
            break;
        default:
            return false;
        }
    }
    return sensors[type]->init();
}

/**
 * @brief Configures a sensor.
 * @param[in] type Sensor type to configure.
 * @param[in] config A map of configuration parameters.
 * @return True if configuration was successful, false otherwise.
 * @ingroup Sensors
 */
bool SensorWrapper::sensor_configure(SensorType type, const std::map<std::string, std::string> &config)
{
    if (sensors.find(type) == sensors.end())
    {
        return false;
    }
    return sensors[type]->configure(config);
}

/**
 * @brief Reads data from a sensor.
 * @param[in] sensorType Sensor type to read from.
 * @param[in] dataType Data type to read.
 * @return The sensor data.
 * @ingroup Sensors
 */
float SensorWrapper::sensor_read_data(SensorType sensorType, SensorDataTypeIdentifier dataType)
{
    if (sensors.find(sensorType) == sensors.end())
    {
        return -1.0f;
    }
    return sensors[sensorType]->read_data(dataType);
}

/**
 * @brief Gets a sensor.
 * @param[in] type Sensor type to get.
 * @return A pointer to the sensor.
 * @ingroup Sensors
 */
ISensor *SensorWrapper::get_sensor(SensorType type)
{
    return sensors[type];
}
/** @} */