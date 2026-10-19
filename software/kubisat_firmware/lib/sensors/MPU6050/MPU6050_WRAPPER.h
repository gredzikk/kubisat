// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef MPU6050_WRAPPER_H
#define MPU6050_WRAPPER_H

#include "ISensor.h"
#include "MPU6050.h"

class MPU6050Wrapper : public ISensor
{
private:
    MPU6050 sensor_;
    bool initialized_ = false;
    float ax_ = 0, ay_ = 0, az_ = 0;
    float gx_ = 0, gy_ = 0, gz_ = 0;

public:
    explicit MPU6050Wrapper(i2c_inst_t *i2c);

    bool init() override;
    float read_data(SensorDataTypeIdentifier type) override;
    bool is_initialized() const override;
    SensorType get_type() const override;
    bool configure(const std::map<std::string, std::string> &config) override;
    uint8_t get_address() const override { return MPU6050::DEFAULT_ADDRESS; }
};

#endif
