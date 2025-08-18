// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "BME280_WRAPPER.h"

BME280Wrapper::BME280Wrapper(i2c_inst_t *i2c) : sensor_(i2c) {}

bool BME280Wrapper::init()
{
    initialized_ = sensor_.init();
    return initialized_;
}

float BME280Wrapper::read_data(SensorDataTypeIdentifier type)
{
    int32_t temp_raw, pressure_raw, humidity_raw;
    sensor_.read_raw_all(&temp_raw, &pressure_raw, &humidity_raw);

    switch (type)
    {
    case SensorDataTypeIdentifier::TEMPERATURE:
        return sensor_.convert_temperature(temp_raw);
    case SensorDataTypeIdentifier::PRESSURE:
        return sensor_.convert_pressure(pressure_raw);
    case SensorDataTypeIdentifier::HUMIDITY:
        return sensor_.convert_humidity(humidity_raw);
    default:
        return 0.0f;
    }
}

bool BME280Wrapper::is_initialized() const
{
    return initialized_;
}

SensorType BME280Wrapper::get_type() const
{
    return SensorType::ENVIRONMENT;
}

bool BME280Wrapper::configure([[maybe_unused]] const std::map<std::string, std::string> &config)
{
    return true;
}