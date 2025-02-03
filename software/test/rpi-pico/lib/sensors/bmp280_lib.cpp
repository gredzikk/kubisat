#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

#define ADDR _u(0x76)
#define REG_CONFIG _u(0xF5)
#define REG_CTRL_MEAS _u(0xF4)
#define REG_RESET _u(0xE0)
#define REG_TEMP_XLSB _u(0xFC)
#define REG_TEMP_LSB _u(0xFB)
#define REG_TEMP_MSB _u(0xFA)
#define REG_PRESSURE_XLSB _u(0xF9)
#define REG_PRESSURE_LSB _u(0xF8)
#define REG_PRESSURE_MSB _u(0xF7)
#define REG_DIG_T1_LSB _u(0x88)
#define REG_DIG_T1_MSB _u(0x89)
#define REG_DIG_T2_LSB _u(0x8A)
#define REG_DIG_T2_MSB _u(0x8B)
#define REG_DIG_T3_LSB _u(0x8C)
#define REG_DIG_T3_MSB _u(0x8D)
#define REG_DIG_P1_LSB _u(0x8E)
#define REG_DIG_P1_MSB _u(0x8F)
#define REG_DIG_P2_LSB _u(0x90)
#define REG_DIG_P2_MSB _u(0x91)
#define REG_DIG_P3_LSB _u(0x92)
#define REG_DIG_P3_MSB _u(0x93)
#define REG_DIG_P4_LSB _u(0x94)
#define REG_DIG_P4_MSB _u(0x95)
#define REG_DIG_P5_LSB _u(0x96)
#define REG_DIG_P5_MSB _u(0x97)
#define REG_DIG_P6_LSB _u(0x98)
#define REG_DIG_P6_MSB _u(0x99)
#define REG_DIG_P7_LSB _u(0x9A)
#define REG_DIG_P7_MSB _u(0x9B)
#define REG_DIG_P8_LSB _u(0x9C)
#define REG_DIG_P8_MSB _u(0x9D)
#define REG_DIG_P9_LSB _u(0x9E)
#define REG_DIG_P9_MSB _u(0x9F)
#define NUM_CALIB_PARAMS 24

class BMP280 {
public:
    struct CalibParam {
        uint16_t dig_t1;
        int16_t dig_t2;
        int16_t dig_t3;
        uint16_t dig_p1;
        int16_t dig_p2;
        int16_t dig_p3;
        int16_t dig_p4;
        int16_t dig_p5;
        int16_t dig_p6;
        int16_t dig_p7;
        int16_t dig_p8;
        int16_t dig_p9;
    };

    BMP280() {
        bmp280_init();
        bmp280_get_calib_params();
    }

    void read_raw(int32_t& temp, int32_t& pressure) {
        uint8_t buf[6];
        uint8_t reg = REG_PRESSURE_MSB;
        i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);
        i2c_read_blocking(i2c_default, ADDR, buf, 6, false);
        pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
        temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
    }

    int32_t convert_temp(int32_t temp) {
        int32_t t_fine = bmp280_convert(temp);
        return (t_fine * 5 + 128) >> 8;
    }

    int32_t convert_pressure(int32_t pressure, int32_t temp) {
        int32_t t_fine = bmp280_convert(temp);
        int32_t var1, var2;
        uint32_t converted = 0.0;
        var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
        var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)params.dig_p6);
        var2 += ((var1 * ((int32_t)params.dig_p5)) << 1);
        var2 = (var2 >> 2) + (((int32_t)params.dig_p4) << 16);
        var1 = (((params.dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)params.dig_p2) * var1) >> 1)) >> 18;
        var1 = ((((32768 + var1)) * ((int32_t)params.dig_p1)) >> 15);
        if (var1 == 0) {
            return 0;
        }
        converted = (((uint32_t)(((int32_t)1048576) - pressure) - (var2 >> 12))) * 3125;
        if (converted < 0x80000000) {
            converted = (converted << 1) / ((uint32_t)var1);
        } else {
            converted = (converted / (uint32_t)var1) * 2;
        }
        var1 = (((int32_t)params.dig_p9) * ((int32_t)(((converted >> 3) * (converted >> 3)) >> 13))) >> 12;
        var2 = (((int32_t)(converted >> 2)) * ((int32_t)params.dig_p8)) >> 13;
        converted = (uint32_t)((int32_t)converted + ((var1 + var2 + params.dig_p7) >> 4));
        return converted;
    }

private:
    CalibParam params;

    void bmp280_init() {
        uint8_t buf[2];
        const uint8_t reg_config_val = ((0x04 << 5) | (0x05 << 2)) & 0xFC;
        buf[0] = REG_CONFIG;
        buf[1] = reg_config_val;
        i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
        const uint8_t reg_ctrl_meas_val = (0x01 << 5) | (0x03 << 2) | (0x03);
        buf[0] = REG_CTRL_MEAS;
        buf[1] = reg_ctrl_meas_val;
        i2c_write_blocking(i2c_default, ADDR, buf, 2, false);
    }

    int32_t bmp280_convert(int32_t temp) {
        int32_t var1, var2;
        var1 = ((((temp >> 3) - ((int32_t)params.dig_t1 << 1))) * ((int32_t)params.dig_t2)) >> 11;
        var2 = (((((temp >> 4) - ((int32_t)params.dig_t1)) * ((temp >> 4) - ((int32_t)params.dig_t1))) >> 12) * ((int32_t)params.dig_t3)) >> 14;
        return var1 + var2;
    }

    void bmp280_get_calib_params() {
        uint8_t buf[NUM_CALIB_PARAMS] = { 0 };
        uint8_t reg = REG_DIG_T1_LSB;
        i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);
        i2c_read_blocking(i2c_default, ADDR, buf, NUM_CALIB_PARAMS, false);
        params.dig_t1 = (uint16_t)(buf[1] << 8) | buf[0];
        params.dig_t2 = (int16_t)(buf[3] << 8) | buf[2];
        params.dig_t3 = (int16_t)(buf[5] << 8) | buf[4];
        params.dig_p1 = (uint16_t)(buf[7] << 8) | buf[6];
        params.dig_p2 = (int16_t)(buf[9] << 8) | buf[8];
        params.dig_p3 = (int16_t)(buf[11] << 8) | buf[10];
        params.dig_p4 = (int16_t)(buf[13] << 8) | buf[12];
        params.dig_p5 = (int16_t)(buf[15] << 8) | buf[14];
        params.dig_p6 = (int16_t)(buf[17] << 8) | buf[16];
        params.dig_p7 = (int16_t)(buf[19] << 8) | buf[18];
        params.dig_p8 = (int16_t)(buf[21] << 8) | buf[20];
        params.dig_p9 = (int16_t)(buf[23] << 8) | buf[22];
    }
};
