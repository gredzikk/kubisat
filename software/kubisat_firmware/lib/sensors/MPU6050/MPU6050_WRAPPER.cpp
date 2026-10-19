// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "MPU6050_WRAPPER.h"

MPU6050Wrapper::MPU6050Wrapper(i2c_inst_t *i2c) : sensor_(i2c) {}

bool MPU6050Wrapper::init()
{
    initialized_ = sensor_.begin();
    return initialized_;
}

float MPU6050Wrapper::read_data(SensorDataTypeIdentifier type)
{
    if (!initialized_)
    {
        return -1.0f;
    }

    switch (type)
    {
    case SensorDataTypeIdentifier::ACCEL_X:
    case SensorDataTypeIdentifier::ACCEL_Y:
    case SensorDataTypeIdentifier::ACCEL_Z:
        if (!sensor_.read_accel_g(ax_, ay_, az_))
        {
            return -1.0f;
        }
        if (type == SensorDataTypeIdentifier::ACCEL_X)
            return ax_;
        if (type == SensorDataTypeIdentifier::ACCEL_Y)
            return ay_;
        return az_;
    case SensorDataTypeIdentifier::GYRO_X:
    case SensorDataTypeIdentifier::GYRO_Y:
    case SensorDataTypeIdentifier::GYRO_Z:
        if (!sensor_.read_gyro_dps(gx_, gy_, gz_))
        {
            return -1.0f;
        }
        if (type == SensorDataTypeIdentifier::GYRO_X)
            return gx_;
        if (type == SensorDataTypeIdentifier::GYRO_Y)
            return gy_;
        return gz_;
    case SensorDataTypeIdentifier::TEMPERATURE:
    {
        float t = 0;
        if (!sensor_.read_temp_c(t))
        {
            return -1.0f;
        }
        return t;
    }
    default:
        return -1.0f;
    }
}

bool MPU6050Wrapper::is_initialized() const
{
    return initialized_;
}

SensorType MPU6050Wrapper::get_type() const
{
    return SensorType::IMU;
}

bool MPU6050Wrapper::configure(const std::map<std::string, std::string> &)
{
    return initialized_;
}
