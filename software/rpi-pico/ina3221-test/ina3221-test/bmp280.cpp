#include "bmp280.h"

#include <iomanip>
#include <vector>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

// BMP280 Class Implementation

BMP280::BMP280(i2c_inst_t* i2cPort, uint8_t address)
    : i2c_port(i2cPort), device_addr(address), calib_params{}, initialized(false), t_fine(0) {
}

bool BMP280::init() {
    if (!i2c_port) {
        std::cerr << "Invalid I2C port.\n";
        return false;
    }

    // Configure BMP280
    if (!configure_sensor()) {
        std::cerr << "Failed to configure BMP280 sensor.\n";
        return false;
    }

    // Retrieve calibration parameters
    if (!get_calibration_parameters()) {
        std::cerr << "Failed to retrieve calibration parameters.\n";
        return false;
    }

    initialized = true;
    std::cout << "BMP280 sensor initialized successfully.\n";
    return true;
}

void BMP280::reset() {
    uint8_t buf[2] = { REG_RESET, 0xB6 };
    int ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to reset BMP280 sensor.\n";
    }
    sleep_ms(10); // Wait for reset to complete
}

bool BMP280::read_raw(int32_t* temperature, int32_t* pressure) {
    if (!initialized) {
        std::cerr << "BMP280 not initialized.\n";
        return false;
    }

    uint8_t buf[6];
    uint8_t reg = REG_PRESSURE_MSB;
    int ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write register address to BMP280.\n";
        return false;
    }

    ret = i2c_read_blocking(i2c_port, device_addr, buf, 6, false);
    if (ret != 6) {
        std::cerr << "Failed to read data from BMP280.\n";
        return false;
    }

    // Combine the bytes to form raw temperature and pressure values
    *pressure = (static_cast<int32_t>(buf[0]) << 12) | (static_cast<int32_t>(buf[1]) << 4) | (static_cast<int32_t>(buf[2] >> 4));
    *temperature = (static_cast<int32_t>(buf[3]) << 12) | (static_cast<int32_t>(buf[4]) << 4) | (static_cast<int32_t>(buf[5] >> 4));

    return true;
}

float BMP280::convert_temperature(int32_t temp_raw) const {
    // Apply compensation as per datasheet
    int32_t var1, var2;
    var1 = ((((temp_raw >> 3) - ((int32_t)calib_params.dig_t1 << 1))) * ((int32_t)calib_params.dig_t2)) >> 11;
    var2 = (((((temp_raw >> 4) - ((int32_t)calib_params.dig_t1)) *
              ((temp_raw >> 4) - ((int32_t)calib_params.dig_t1))) >> 12) *
            ((int32_t)calib_params.dig_t3)) >> 14;
    int32_t t_fine_local = var1 + var2;
    float temperature = (t_fine_local * 5 + 128) >> 8;
    return temperature / 100.0f;
}

float BMP280::convert_pressure(int32_t pressure_raw, int32_t temp_raw) const {
    // Apply compensation as per datasheet
    int32_t var1, var2;
    var1 = (t_fine = ((((temp_raw >> 3) - ((int32_t)calib_params.dig_t1 << 1))) *
                      ((int32_t)calib_params.dig_t2)) >>
                     11) +
           (((((temp_raw >> 4) - ((int32_t)calib_params.dig_t1)) *
              ((temp_raw >> 4) - ((int32_t)calib_params.dig_t1))) >>
             12) *
            ((int32_t)calib_params.dig_t3)) >>
               14;
    var2 = ((var1 >> 1) * (var1 >> 1)) >> 12;
    var2 *= ((int32_t)calib_params.dig_p6) << 11;
    var2 += ((var1 * ((int32_t)calib_params.dig_p5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)calib_params.dig_p4) << 16);
    var1 = (((calib_params.dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) +
            ((((int32_t)calib_params.dig_p2) * var1) >> 1)) >>
           18;
    var1 = ((((32768 + var1)) * ((int32_t)calib_params.dig_p1)) >> 15);
    if (var1 == 0) {
        return 0; // Avoid division by zero
    }
    uint32_t converted = (((uint32_t)((1048576 - pressure_raw) - (var2 >> 12))) * 3125);
    if (converted < 0x80000000) {
        converted = (converted << 1) / static_cast<uint32_t>(var1);
    } else {
        converted = (converted / static_cast<uint32_t>(var1)) * 2;
    }
    var1 = (((int32_t)calib_params.dig_p9) *
            ((int32_t)(((converted >> 3) * (converted >> 3)) >> 13))) >>
           12;
    var2 = (((int32_t)(converted >> 2)) * ((int32_t)calib_params.dig_p8)) >> 13;
    converted = static_cast<uint32_t>(((int32_t)converted + ((var1 + var2 + calib_params.dig_p7) >> 4)));
    return static_cast<float>(converted) / 100.0f;
}

bool BMP280::get_calibration_parameters() {
    uint8_t buf[NUM_CALIB_PARAMS] = {0};
    uint8_t reg = REG_DIG_T1_LSB;
    int ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write calibration register address to BMP280.\n";
        return false;
    }

    ret = i2c_read_blocking(i2c_port, device_addr, buf, NUM_CALIB_PARAMS, false);
    if (ret != NUM_CALIB_PARAMS) {
        std::cerr << "Failed to read calibration parameters from BMP280.\n";
        return false;
    }

    // Store calibration parameters
    calib_params.dig_t1 = (static_cast<uint16_t>(buf[1]) << 8) | buf[0];
    calib_params.dig_t2 = static_cast<int16_t>((buf[3] << 8) | buf[2]);
    calib_params.dig_t3 = static_cast<int16_t>((buf[5] << 8) | buf[4]);

    calib_params.dig_p1 = (static_cast<uint16_t>(buf[7]) << 8) | buf[6];
    calib_params.dig_p2 = static_cast<int16_t>((buf[9] << 8) | buf[8]);
    calib_params.dig_p3 = static_cast<int16_t>((buf[11] << 8) | buf[10]);
    calib_params.dig_p4 = static_cast<int16_t>((buf[13] << 8) | buf[12]);
    calib_params.dig_p5 = static_cast<int16_t>((buf[15] << 8) | buf[14]);
    calib_params.dig_p6 = static_cast<int16_t>((buf[17] << 8) | buf[16]);
    calib_params.dig_p7 = static_cast<int16_t>((buf[19] << 8) | buf[18]);
    calib_params.dig_p8 = static_cast<int16_t>((buf[21] << 8) | buf[20]);
    calib_params.dig_p9 = static_cast<int16_t>((buf[23] << 8) | buf[22]);

    return true;
}

bool BMP280::configure_sensor() {
    uint8_t buf[2];

    // 500ms sampling time, x16 filter
    const uint8_t reg_config_val = ((0x04 << 5) | (0x05 << 2)) & 0xFC;

    buf[0] = REG_CONFIG;
    buf[1] = reg_config_val;
    int ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CONFIG register to BMP280.\n";
        return false;
    }

    // OSRS_T x1, OSRS_P x4, normal mode operation
    const uint8_t reg_ctrl_meas_val = (0x01 << 5) | (0x03 << 2) | (0x03);
    buf[0] = REG_CTRL_MEAS;
    buf[1] = reg_ctrl_meas_val;
    ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CTRL_MEAS register to BMP280.\n";
        return false;
    }

    return true;
}