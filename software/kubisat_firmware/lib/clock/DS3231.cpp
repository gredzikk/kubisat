// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "DS3231.h"
#include "utils.h"
#include <cstdio>
#include <mutex>
#include "event_manager.h"
#include "NMEA_data.h"

/**
 * @defgroup DS3231 RTC clock
 * @brief Functions for interfacing with the DS3231 RTC module
 * @{
 */

/**
 * @brief Constructor for the DS3231 class
 *
 * @details Initializes the I2C interface and sets the device address for the DS3231 RTC module.
 *         The constructor is private to enforce the singleton pattern, ensuring that only one
 *        instance of the class can be created. The mutex for the class is also initialized.
 * @note The DS3231 device address is defined in the header file as DS3231_DEVICE_ADRESS.
 * @ingroup DS3231_RTC
 */
DS3231::DS3231() : i2c(MAIN_I2C_PORT), ds3231_addr(DS3231_DEVICE_ADRESS)
{
    recursive_mutex_init(&clock_mutex_);
}

/**
 * @brief Gets the singleton instance of the DS3231 class
 *
 * @return A reference to the singleton instance of the DS3231 class
 * @details This function provides access to the single instance of the DS3231
 *          class, ensuring that only one object manages the RTC module. The
 *          instance is created upon the first call to this function and remains
 *          available for the lifetime of the program.
 * @ingroup DS3231_RTC
 */
DS3231 &DS3231::get_instance()
{
    static DS3231 instance;
    return instance;
}

/**
 * @brief Gets current RTC time as Unix timestamp
 * @return Unix timestamp or -1 on error
 */
time_t DS3231::get_time()
{
    uint8_t time_data[7];
    if (i2c_read_reg(DS3231_SECONDS_REG, 7, time_data) != 0)
    {
        uart_print("Failed to read time from RTC", VerbosityLevel::ERROR);
        return -1;
    }

    struct tm timeinfo = {};
    // Convert BCD to binary and fill tm structure
    timeinfo.tm_sec = ((time_data[0] >> 4) * 10) + (time_data[0] & 0x0F);
    timeinfo.tm_min = ((time_data[1] >> 4) * 10) + (time_data[1] & 0x0F);
    timeinfo.tm_hour = ((time_data[2] >> 4) * 10) + (time_data[2] & 0x0F);
    timeinfo.tm_mday = ((time_data[4] >> 4) * 10) + (time_data[4] & 0x0F);
    timeinfo.tm_mon = (((time_data[5] & 0x1F) >> 4) * 10) + (time_data[5] & 0x0F) - 1;
    timeinfo.tm_year = ((time_data[6] >> 4) * 10) + (time_data[6] & 0x0F) + 100;
    timeinfo.tm_isdst = 0;

    time_t unix_time = mktime(&timeinfo);
    if (unix_time == -1)
    {
        uart_print("Failed to convert RTC time", VerbosityLevel::ERROR);
        return -1;
    }

    return unix_time;
}

/**
 * @brief Sets the RTC time using Unix timestamp
 * @param unix_time Time in seconds since Unix epoch
 * @return 0 on success, -1 on failure
 */
int DS3231::set_time(time_t unix_time)
{
    struct tm *timeinfo = gmtime(&unix_time);
    if (!timeinfo)
    {
        uart_print("Failed to convert Unix time", VerbosityLevel::ERROR);
        return -1;
    }

    uint8_t time_data[7];
    // Convert directly to BCD
    time_data[0] = ((timeinfo->tm_sec / 10) << 4) | (timeinfo->tm_sec % 10);
    time_data[1] = ((timeinfo->tm_min / 10) << 4) | (timeinfo->tm_min % 10);
    time_data[2] = ((timeinfo->tm_hour / 10) << 4) | (timeinfo->tm_hour % 10);
    time_data[3] = timeinfo->tm_wday == 0 ? 7 : timeinfo->tm_wday;
    time_data[4] = ((timeinfo->tm_mday / 10) << 4) | (timeinfo->tm_mday % 10);
    time_data[5] = (((timeinfo->tm_mon + 1) / 10) << 4) | ((timeinfo->tm_mon + 1) % 10);
    time_data[6] = (((timeinfo->tm_year - 100) / 10) << 4) | ((timeinfo->tm_year - 100) % 10);

    if (i2c_write_reg(DS3231_SECONDS_REG, 7, time_data) != 0)
    {
        uart_print("Failed to write time to RTC", VerbosityLevel::ERROR);

        return -1;
    }

    return 0;
}

/**
 * @brief Reads the temperature from the DS3231's internal temperature sensor
 *
 * @param[out] resolution Pointer to a float to store the temperature value
 * @return 0 on success, -1 on failure
 * @details The DS3231 includes an internal temperature sensor with 0.25°C resolution.
 *          This function reads the sensor value and calculates the temperature in
 *          degrees Celsius. The temperature sensor is primarily used for the
 *          oscillator's temperature compensation, but can be used for general
 *          temperature monitoring as well.
 * @ingroup DS3231_RTC
 */
int DS3231::read_temperature(float *resolution)
{
    std::string status;
    uint8_t temp[2];
    int result = i2c_read_reg(DS3231_TEMPERATURE_MSB_REG, 2, temp);
    if (result != 0)
    {
        status = "Failed to read temperature from DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        return -1;
    }

    int8_t temperature_msb = (int8_t)temp[0];
    uint8_t temperature_lsb = temp[1] >> 6; // Only the 2 MSB are valid

    *resolution = temperature_msb + (temperature_lsb * 0.25f); // 0.25 degree resolution

    return 0;
}

/**
 * @brief Gets the currently configured timezone offset
 *
 * @return The timezone offset in minutes
 * @details Returns the current timezone offset in minutes relative to UTC.
 *          Positive values represent timezones ahead of UTC (east),
 *          negative values represent timezones behind UTC (west).
 * @ingroup DS3231_RTC
 */
int16_t DS3231::get_timezone_offset() const
{
    return timezone_offset_minutes_;
}

/**
 * @brief Sets the timezone offset
 *
 * @param[in] offset_minutes The timezone offset in minutes
 * @details Sets the timezone offset in minutes relative to UTC. This value
 *          is used when converting between UTC and local time. The function
 *          validates that the offset is within a valid range (-720 to +720 minutes,
 *          which corresponds to -12 to +12 hours).
 * @note This setting is stored in memory and does not persist across reboots.
 * @ingroup DS3231_RTC
 */
void DS3231::set_timezone_offset(int16_t offset_minutes)
{
    // Validate range: -12 hours to +12 hours (-720 to +720 minutes)
    if (offset_minutes >= -720 && offset_minutes <= 720)
    {
        timezone_offset_minutes_ = offset_minutes;
    }
    else
    {
        uart_print("Error: Invalid timezone offset", VerbosityLevel::ERROR);
    }
}

/**
 * @brief Gets the current local time by applying the timezone offset to UTC time
 *
 * @return Local time as Unix timestamp, or -1 on error
 * @details Retrieves the current UTC time from the RTC and applies the configured
 *          timezone offset (in minutes) to calculate the local time.
 * @ingroup DS3231_RTC
 */
time_t DS3231::get_local_time()
{
    time_t utc_time = get_time();
    if (utc_time == -1)
    {
        return -1;
    }

    return utc_time + (timezone_offset_minutes_ * 60);
}

// ==================== private methods

/**
 * @brief Reads data from a specific register on the DS3231
 *
 * @param[in] reg_addr Register address to read from
 * @param[in] length Number of bytes to read
 * @param[out] data Buffer to store read data
 * @return 0 on success, -1 on failure
 * @details This method performs a thread-safe I²C read operation from the DS3231.
 *          It first writes the register address to the device, then reads the
 *          requested number of bytes. All access is protected by a mutex to prevent
 *          concurrent I²C operations that could corrupt data.
 *
 * @note This is a low-level method used internally by the class.
 * @ingroup DS3231_RTC
 */
int DS3231::i2c_read_reg(uint8_t reg_addr, size_t length, uint8_t *data)
{
    if (!length)
        return -1;
    std::string status;

    recursive_mutex_enter_blocking(&clock_mutex_);
    uint8_t reg = reg_addr;
    int write_result = i2c_write_blocking(i2c, ds3231_addr, &reg, 1, true);
    if (write_result == PICO_ERROR_GENERIC)
    {
        status = "Failed to write register address to DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        recursive_mutex_exit(&clock_mutex_);
        return -1;
    }
    int read_result = i2c_read_blocking(i2c, ds3231_addr, data, length, false);
    if (read_result == PICO_ERROR_GENERIC)
    {
        status = "Failed to read register data from DS3231";
        uart_print(status, VerbosityLevel::ERROR);
        recursive_mutex_exit(&clock_mutex_);
        return -1;
    }
    recursive_mutex_exit(&clock_mutex_);

    return 0;
}

/**
 * @brief Writes data to a specific register on the DS3231
 *
 * @param[in] reg_addr Register address to write to
 * @param[in] length Number of bytes to write
 * @param[in] data Buffer containing data to write
 * @return 0 on success, -1 on failure
 * @details This method performs a thread-safe I²C write operation to the DS3231.
 *          It combines the register address and data into a single buffer and
 *          sends it to the device. All access is protected by a mutex to prevent
 *          concurrent I²C operations that could corrupt data.
 *
 * @note This is a low-level method used internally by the class.
 * @ingroup DS3231_RTC
 */
int DS3231::i2c_write_reg(uint8_t reg_addr, size_t length, uint8_t *data)
{
    if (!length)
        return -1;

    recursive_mutex_enter_blocking(&clock_mutex_);
    std::vector<uint8_t> message(length + 1);
    message[0] = reg_addr;
    for (size_t i = 0; i < length; i++)
    {
        message[i + 1] = data[i];
    }
    int write_result = i2c_write_blocking(i2c, ds3231_addr, message.data(), (length + 1), false);
    if (write_result == PICO_ERROR_GENERIC)
    {
        uart_print("Error: i2c_write_blocking failed in i2c_write_reg", VerbosityLevel::ERROR);
        recursive_mutex_exit(&clock_mutex_);
        return -1;
    }
    recursive_mutex_exit(&clock_mutex_);

    return 0;
}
/** @} */ // End of DS3231_RTC group