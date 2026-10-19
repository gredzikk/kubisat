// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "MPU6050.h"

static constexpr uint8_t REG_PWR_MGMT_1 = 0x6B;
static constexpr uint8_t REG_WHO_AM_I = 0x75;
static constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
static constexpr uint8_t REG_TEMP_OUT_H = 0x41;
static constexpr uint8_t REG_GYRO_XOUT_H = 0x43;

MPU6050::MPU6050(i2c_inst_t *i2c, uint8_t address) : i2c_(i2c), address_(address) {}

bool MPU6050::write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    return i2c_write_blocking(i2c_, address_, buf, 2, false) == 2;
}

bool MPU6050::read_regs(uint8_t reg, uint8_t *buffer, size_t length)
{
    if (i2c_write_blocking(i2c_, address_, &reg, 1, true) != 1)
    {
        return false;
    }
    return i2c_read_blocking(i2c_, address_, buffer, length, false) == static_cast<int>(length);
}

bool MPU6050::begin()
{
    uint8_t who = 0;
    if (!read_regs(REG_WHO_AM_I, &who, 1))
    {
        return false;
    }
    // Fail closed: only known MPU-6050 / clone WHO_AM_I values
    if (who != 0x68 && who != 0x70 && who != 0x71)
    {
        return false;
    }
    // Wake device
    return write_reg(REG_PWR_MGMT_1, 0x00);
}

bool MPU6050::read_accel_g(float &ax, float &ay, float &az)
{
    uint8_t raw[6];
    if (!read_regs(REG_ACCEL_XOUT_H, raw, 6))
    {
        return false;
    }
    int16_t x = static_cast<int16_t>((raw[0] << 8) | raw[1]);
    int16_t y = static_cast<int16_t>((raw[2] << 8) | raw[3]);
    int16_t z = static_cast<int16_t>((raw[4] << 8) | raw[5]);
    constexpr float scale = 16384.0f; // ±2g default
    ax = static_cast<float>(x) / scale;
    ay = static_cast<float>(y) / scale;
    az = static_cast<float>(z) / scale;
    return true;
}

bool MPU6050::read_gyro_dps(float &gx, float &gy, float &gz)
{
    uint8_t raw[6];
    if (!read_regs(REG_GYRO_XOUT_H, raw, 6))
    {
        return false;
    }
    int16_t x = static_cast<int16_t>((raw[0] << 8) | raw[1]);
    int16_t y = static_cast<int16_t>((raw[2] << 8) | raw[3]);
    int16_t z = static_cast<int16_t>((raw[4] << 8) | raw[5]);
    constexpr float scale = 131.0f; // ±250 dps default
    gx = static_cast<float>(x) / scale;
    gy = static_cast<float>(y) / scale;
    gz = static_cast<float>(z) / scale;
    return true;
}

bool MPU6050::read_temp_c(float &temp_c)
{
    uint8_t raw[2];
    if (!read_regs(REG_TEMP_OUT_H, raw, 2))
    {
        return false;
    }
    int16_t t = static_cast<int16_t>((raw[0] << 8) | raw[1]);
    temp_c = (static_cast<float>(t) / 340.0f) + 36.53f;
    return true;
}
