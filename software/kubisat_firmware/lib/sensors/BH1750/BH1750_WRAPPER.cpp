// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "BH1750_WRAPPER.h"
#include <string>
#include <iostream>

BH1750Wrapper::BH1750Wrapper(i2c_inst_t *i2c) : sensor_(i2c)
{
    sensor_.configure(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE);
}

bool BH1750Wrapper::init()
{
    initialized_ = sensor_.begin();
    return initialized_;
}

float BH1750Wrapper::read_data(SensorDataTypeIdentifier type)
{
    if (type == SensorDataTypeIdentifier::LIGHT_LEVEL)
    {
        return sensor_.get_light_level();
    }
    return 0.0f;
}

bool BH1750Wrapper::is_initialized() const
{
    return initialized_;
}

SensorType BH1750Wrapper::get_type() const
{
    return SensorType::LIGHT;
}

bool BH1750Wrapper::configure(const std::map<std::string, std::string> &config)
{
    for (const auto &[key, value] : config)
    {
        if (key == "measurement_mode")
        {
            if (value == "continuously_high_resolution")
            {
                sensor_.configure(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE);
            }
            else if (value == "continuously_high_resolution_2")
            {
                sensor_.configure(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE_2);
            }
            else if (value == "continuously_low_resolution")
            {
                sensor_.configure(BH1750::Mode::CONTINUOUS_LOW_RES_MODE);
            }
            else if (value == "one_time_high_resolution")
            {
                sensor_.configure(BH1750::Mode::ONE_TIME_HIGH_RES_MODE);
            }
            else if (value == "one_time_high_resolution_2")
            {
                sensor_.configure(BH1750::Mode::ONE_TIME_HIGH_RES_MODE_2);
            }
            else if (value == "one_time_low_resolution")
            {
                sensor_.configure(BH1750::Mode::ONE_TIME_LOW_RES_MODE);
            }
            else
            {
                std::cerr << "[BH1750Wrapper] Unknown measurement_mode value: " << value << std::endl;
                return false;
            }
        }
        else
        {
            std::cerr << "[BH1750Wrapper] Unknown configuration key: " << key << std::endl;
            return false;
        }
    }
    return true;
}