// bmp280.cpp

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

bool BMP280::read_raw_all(int32_t* temperature, int32_t* pressure, int32_t* humidity) {
    if (!initialized) {
        std::cerr << "BMP280 not initialized.\n";
        return false;
    }

    // Define the starting register address
    uint8_t start_reg = REG_PRESSURE_MSB;
    // Total bytes to read: 6 (pressure & temperature) + 2 (humidity) = 8
    uint8_t buf[8] = {0};

    // Write the starting register address
    int ret = i2c_write_blocking(i2c_port, device_addr, &start_reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write starting register address to BMP280.\n";
        return false;
    }

    // Read 8 bytes: pressure (3), temperature (3), humidity (2)
    ret = i2c_read_blocking(i2c_port, device_addr, buf, 8, false);
    if (ret != 8) {
        std::cerr << "Failed to read all data from BMP280.\n";
        return false;
    }

    // Combine the bytes to form raw pressure and temperature values
    *pressure = (static_cast<int32_t>(buf[0]) << 12) |
               (static_cast<int32_t>(buf[1]) << 4) |
               (static_cast<int32_t>(buf[2] >> 4));
    *temperature = (static_cast<int32_t>(buf[3]) << 12) |
                  (static_cast<int32_t>(buf[4]) << 4) |
                  (static_cast<int32_t>(buf[5] >> 4));
    *humidity = (static_cast<int32_t>(buf[6]) << 8) |
               static_cast<int32_t>(buf[7]);

    return true;
}

float BMP280::convert_temperature(int32_t temp_raw) const {
    // Apply compensation as per datasheet
    int32_t var1, var2;
    var1 = ((((temp_raw >> 3) - ((int32_t)calib_params.dig_t1 << 1))) *
            ((int32_t)calib_params.dig_t2)) >> 11;
    var2 = (((((temp_raw >> 4) - ((int32_t)calib_params.dig_t1)) *
              ((temp_raw >> 4) - ((int32_t)calib_params.dig_t1))) >> 12) *
            ((int32_t)calib_params.dig_t3)) >> 14;
    t_fine = var1 + var2;
    float temperature = (t_fine * 5 + 128) >> 8;
    return temperature / 100.0f;
}

float BMP280::convert_pressure(int32_t pressure_raw, int32_t temp_raw) const {
    // Apply compensation as per datasheet
    // Temperature compensation should already be done
    int32_t var1, var2;
    var1 = t_fine - 128000;
    var2 = var1 * var1 * calib_params.dig_p6;
    var2 = var2 + ((var1 * calib_params.dig_p5) << 17);
    var2 = var2 + ((int32_t)calib_params.dig_p4 << 35);
    var1 = ((var1 * var1 * calib_params.dig_p3) >> 8) +
           ((var1 * calib_params.dig_p2) << 12);
    var1 = (((1LL << 47) + var1)) * calib_params.dig_p1 >> 33;

    if (var1 == 0) {
        return 0; // Avoid division by zero
    }

    int64_t p = 1048576 - pressure_raw;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (calib_params.dig_p9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = (calib_params.dig_p8 * p) >> 19;

    p = ((p + var1 + var2) >> 8) + ((int64_t)calib_params.dig_p7 << 4);
    return (float)p / 256.0f;
}

float BMP280::convert_humidity(int32_t humidity_raw) const {
    // Apply compensation as per datasheet
    if (t_fine == 0) {
        return 0.0f; // Temperature not read yet
    }

    int32_t var_h;
    var_h = t_fine - 76800;
    var_h = (((((humidity_raw << 14) - ((int32_t)calib_params.dig_h4 << 20) -
                ((int32_t)calib_params.dig_h5 * var_h)) +
               16384) >> 15) *
             (((((((var_h * calib_params.dig_h6) >> 10) *
                   (((var_h * calib_params.dig_h3) >> 11) + 32768)) >>
                  10) +
                2097152) *
               calib_params.dig_h2 + 8192) >>
              14));
    var_h = var_h - (((((var_h >> 15) * (var_h >> 15)) >> 7) *
                     calib_params.dig_h1) >>
                    4);
    if (var_h < 0) {
        var_h = 0;
    }
    if (var_h > 419430400) {
        var_h = 419430400;
    }
    float humidity = (var_h >> 12);
    return humidity / 1024.0f;
}

bool BMP280::get_calibration_parameters() {
    // Total bytes to read: 24 (temp & pressure) + 8 (humidity) = 32
    uint8_t buf[32] = {0};
    uint8_t reg = REG_DIG_T1_LSB;
    int ret = i2c_write_blocking(i2c_port, device_addr, &reg, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write calibration register address to BMP280.\n";
        return false;
    }

    ret = i2c_read_blocking(i2c_port, device_addr, buf, 32, false);
    if (ret != 32) {
        std::cerr << "Failed to read calibration parameters from BMP280.\n";
        return false;
    }

    // Store temperature and pressure calibration parameters
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

    // Store humidity calibration parameters
    calib_params.dig_h1 = buf[24];
    calib_params.dig_h2 = static_cast<int16_t>((buf[26] << 8) | buf[25]);
    calib_params.dig_h3 = buf[27];
    calib_params.dig_h4 = (static_cast<int16_t>(buf[28]) << 4) | (buf[29] & 0x0F);
    calib_params.dig_h5 = (static_cast<int16_t>(buf[30]) << 4) | (buf[31] >> 4);
    calib_params.dig_h6 = static_cast<int8_t>(buf[31] >> 4); // Adjust according to datasheet

    return true;
}

bool BMP280::configure_sensor() {
    uint8_t buf[3];

    // Set humidity oversampling first
    buf[0] = REG_CTRL_HUM;
    buf[1] = 0x03; // Humidity oversampling x4
    int ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CTRL_HUM register to BMP280.\n";
        return false;
    }

    // Configure BMP280
    buf[0] = REG_CONFIG;
    buf[1] = ((0x04 << 5) | (0x05 << 2)) & 0xFC; // 500ms standby, filter x16
    ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CONFIG register to BMP280.\n";
        return false;
    }

    // OSRS_T x1, OSRS_P x4, normal mode operation
    buf[0] = REG_CTRL_MEAS;
    buf[1] = (0x01 << 5) | (0x04 << 2) | (0x03);
    ret = i2c_write_blocking(i2c_port, device_addr, buf, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to write CTRL_MEAS register to BMP280.\n";
        return false;
    }

    return true;
}