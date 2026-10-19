// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef MPU6050_H
#define MPU6050_H

#include "hardware/i2c.h"
#include <cstdint>
#include <cstddef>

class MPU6050
{
public:
    static constexpr uint8_t DEFAULT_ADDRESS = 0x68;

    explicit MPU6050(i2c_inst_t *i2c, uint8_t address = DEFAULT_ADDRESS);

    bool begin();
    bool read_accel_g(float &ax, float &ay, float &az);
    bool read_gyro_dps(float &gx, float &gy, float &gz);
    bool read_temp_c(float &temp_c);

private:
    i2c_inst_t *i2c_;
    uint8_t address_;

    bool write_reg(uint8_t reg, uint8_t value);
    bool read_regs(uint8_t reg, uint8_t *buffer, size_t length);
};

#endif
