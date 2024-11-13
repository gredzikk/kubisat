// BME280.cpp

#include "BME280.h"

#include <iomanip>
#include <vector>
#include <algorithm>
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

// BME280 (BME280) Class Implementation

BME280::BME280(i2c_inst_t* i2cPort, uint8_t address)
    : i2c_port(i2cPort), device_addr(address), calib_params{}, initialized(false), t_fine(0) {
}

bool BME280::init() {
    if (!i2c_port) {
        std::cerr << "Invalid I2C port.\n";
        return false;
    }

    // Check device ID to confirm it's a BME280
    uint8_t reg = 0xD0; // Chip ID register
    uint8_t chip_id = 0;
    int ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write to BME280.\n";
        return false;
    }
    ret = i2c_read_blocking(i2c_port, device_addr, &chip_id, 1, false);
    if (ret != 1) {
        std::cerr << "Failed to read chip ID from BME280.\n";
        return false;
    }
    if (chip_id != 0x60) {
        std::cerr << "Device is not a BME280.\n";
        return false;
    }

    // Configure sensor
    if (!configure_sensor()) {
        std::cerr << "Failed to configure BME280 sensor.\n";
        return false;
    }

    // Retrieve calibration parameters
    if (!get_calibration_parameters()) {
        std::cerr << "Failed to retrieve calibration parameters.\n";
        return false;
    }

    initialized = true;
    std::cout << "BME280 sensor initialized successfully.\n";
    return true;
}

void BME280::reset() {
    uint8_t buf[2] = { REG_RESET, 0xB6 };
    int ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to reset BME280 sensor.\n";
    }
    sleep_ms(10); // Wait for reset to complete
}

bool BME280::read_raw_all(int32_t* temperature, int32_t* pressure, int32_t* humidity) {
    if (!initialized) {
        std::cerr << "BME280 not initialized.\n";
        return false;
    }

    // Define the starting register address
    uint8_t start_reg = REG_PRESSURE_MSB;
    // Total bytes to read: 3 (pressure) + 3 (temperature) + 2 (humidity) = 8
    uint8_t buf[8] = {0};

    // Write the starting register address
    int ret = i2c_write_blocking(i2c_port, device_addr, &start_reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write starting register address to BME280.\n";
        return false;
    }

    // Read data
    ret = i2c_read_blocking(i2c_port, device_addr, buf, 8, false);
    if (ret != 8) {
        std::cerr << "Failed to read data from BME280.\n";
        return false;
    }

    // Combine bytes to form raw values
    *pressure = ((int32_t)buf[0] << 12) | ((int32_t)buf[1] << 4) | ((int32_t)(buf[2] >> 4));
    *temperature = ((int32_t)buf[3] << 12) | ((int32_t)buf[4] << 4) | ((int32_t)(buf[5] >> 4));
    *humidity = ((int32_t)buf[6] << 8) | (int32_t)buf[7];

    return true;
}

float BME280::convert_temperature(int32_t temp_raw) const {
    int32_t var1, var2;
    var1 = ((((temp_raw >> 3) - ((int32_t)calib_params.dig_t1 << 1))) * ((int32_t)calib_params.dig_t2)) >> 11;
    var2 = (((((temp_raw >> 4) - ((int32_t)calib_params.dig_t1)) * ((temp_raw >> 4) - ((int32_t)calib_params.dig_t1))) >> 12) * ((int32_t)calib_params.dig_t3)) >> 14;
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0f;
}

float BME280::convert_pressure(int32_t pressure_raw) const {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_params.dig_p6;
    var2 = var2 + ((var1 * (int64_t)calib_params.dig_p5) << 17);
    var2 = var2 + (((int64_t)calib_params.dig_p4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_params.dig_p3) >> 8) + ((var1 * (int64_t)calib_params.dig_p2) << 12);
    var1 = ((((int64_t)1 << 47) + var1)) * ((int64_t)calib_params.dig_p1) >> 33;

    if (var1 == 0) {
        return 0.0f; // avoid exception caused by division by zero
    }
    p = 1048576 - pressure_raw;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_params.dig_p9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_params.dig_p8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_params.dig_p7) << 4);
    return (float)p / 25600.0f; // in hPa
}

float BME280::convert_humidity(int32_t humidity_raw) const {
    int32_t v_x1_u32r;
    v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((humidity_raw << 14) - ((int32_t)calib_params.dig_h4 << 20) - ((int32_t)calib_params.dig_h5 * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * (int32_t)calib_params.dig_h6) >> 10) * (((v_x1_u32r * (int32_t)calib_params.dig_h3) >> 11) + 32768)) >> 10) + 2097152) *
                   (int32_t)calib_params.dig_h2 + 8192) >> 14));
    v_x1_u32r = std::max(v_x1_u32r, (int32_t)0);
    v_x1_u32r = std::min(v_x1_u32r, (int32_t)419430400);
    float h = v_x1_u32r >> 12;
    return h / 1024.0f;
}

bool BME280::get_calibration_parameters() {
    // Read temperature and pressure calibration data (0x88 to 0xA1)
    uint8_t calib_data[26];
    uint8_t reg = REG_DIG_T1_LSB;
    int ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write to BME280.\n";
        return false;
    }
    ret = i2c_read_blocking(i2c_port, device_addr, calib_data, 26, false);
    if (ret != 26) {
        std::cerr << "Failed to read calibration data from BME280.\n";
        return false;
    }

    // Parse temperature calibration data
    calib_params.dig_t1 = (uint16_t)(calib_data[1] << 8 | calib_data[0]);
    calib_params.dig_t2 = (int16_t)(calib_data[3] << 8 | calib_data[2]);
    calib_params.dig_t3 = (int16_t)(calib_data[5] << 8 | calib_data[4]);

    // Parse pressure calibration data
    calib_params.dig_p1 = (uint16_t)(calib_data[7] << 8 | calib_data[6]);
    calib_params.dig_p2 = (int16_t)(calib_data[9] << 8 | calib_data[8]);
    calib_params.dig_p3 = (int16_t)(calib_data[11] << 8 | calib_data[10]);
    calib_params.dig_p4 = (int16_t)(calib_data[13] << 8 | calib_data[12]);
    calib_params.dig_p5 = (int16_t)(calib_data[15] << 8 | calib_data[14]);
    calib_params.dig_p6 = (int16_t)(calib_data[17] << 8 | calib_data[16]);
    calib_params.dig_p7 = (int16_t)(calib_data[19] << 8 | calib_data[18]);
    calib_params.dig_p8 = (int16_t)(calib_data[21] << 8 | calib_data[20]);
    calib_params.dig_p9 = (int16_t)(calib_data[23] << 8 | calib_data[22]);

    calib_params.dig_h1 = calib_data[25];

    // Read humidity calibration data (0xE1 to 0xE7)
    reg = 0xE1;
    ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write to BME280 for humidity calibration.\n";
        return false;
    }

    uint8_t hum_calib_data[7];
    ret = i2c_read_blocking(i2c_port, device_addr, hum_calib_data, 7, false);
    if (ret != 7) {
        std::cerr << "Failed to read humidity calibration data from BME280.\n";
        return false;
    }

    // Parse humidity calibration data
    calib_params.dig_h2 = (int16_t)(hum_calib_data[1] << 8 | hum_calib_data[0]);
    calib_params.dig_h3 = hum_calib_data[2];
    calib_params.dig_h4 = (int16_t)((hum_calib_data[3] << 4) | (hum_calib_data[4] & 0x0F));
    calib_params.dig_h5 = (int16_t)((hum_calib_data[5] << 4) | (hum_calib_data[4] >> 4));
    calib_params.dig_h6 = (int8_t)hum_calib_data[6];

    return true;
}

bool BME280::configure_sensor() {
    uint8_t buf[2];

    // Set humidity oversampling (must be set before ctrl_meas)
    buf[0] = REG_CTRL_HUM;
    buf[1] = 0x05; // Humidity oversampling x16
    int ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CTRL_HUM to BME280.\n";
        return false;
    }

    // Write config register
    buf[0] = REG_CONFIG;
    buf[1] = 0x00; // Default settings
    ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CONFIG to BME280.\n";
        return false;
    }

    // Write ctrl_meas register
    buf[0] = REG_CTRL_MEAS;
    buf[1] = 0xB7; // Temp and pressure oversampling x16, normal mode
    ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret !=2) {
        std::cerr << "Failed to write CTRL_MEAS to BME280.\n";
        return false;
    }

    return true;
}