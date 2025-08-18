// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef DS3231_H
#define DS3231_H

#include <string>
#include <array>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <time.h>
#include "pico/mutex.h"
#include "lib/location/gps_collector.h"

/**
 * @brief DS3231 I2C device address
 */
#define DS3231_DEVICE_ADRESS 0x68

/**
 * @brief Register address: Seconds (0-59)
 */
#define DS3231_SECONDS_REG 0x00

/**
 * @brief Register address: Minutes (0-59)
 */
#define DS3231_MINUTES_REG 0x01

/**
 * @brief Register address: Hours (0-23 in 24hr mode)
 */
#define DS3231_HOURS_REG 0x02

/**
 * @brief Register address: Day of the week (1-7)
 */
#define DS3231_DAY_REG 0x03

/**
 * @brief Register address: Date (1-31)
 */
#define DS3231_DATE_REG 0x04

/**
 * @brief Register address: Month (1-12) & Century bit
 */
#define DS3231_MONTH_REG 0x05

/**
 * @brief Register address: Year (00-99)
 */
#define DS3231_YEAR_REG 0x06

/**
 * @brief Register address: Control register
 */
#define DS3231_CONTROL_REG 0x0E

/**
 * @brief Register address: Control/Status register
 */
#define DS3231_CONTROL_STATUS_REG 0x0F

/**
 * @brief Register address: Temperature register (MSB)
 */
#define DS3231_TEMPERATURE_MSB_REG 0x11

/**
 * @brief Register address: Temperature register (LSB)
 */
#define DS3231_TEMPERATURE_LSB_REG 0x12

/**
 * @enum days_of_week
 * @brief Enumeration of days of the week
 */
enum days_of_week
{
    MONDAY = 1, ///< Monday
    TUESDAY,    ///< Tuesday
    WEDNESDAY,  ///< Wednesday
    THURSDAY,   ///< Thursday
    FRIDAY,     ///< Friday
    SATURDAY,   ///< Saturday
    SUNDAY      ///< Sunday
};

/**
 * @struct ds3231_data_t
 * @brief Structure to hold time and date information from DS3231
 */
typedef struct
{
    uint8_t seconds; ///< Seconds (0-59)
    uint8_t minutes; ///< Minutes (0-59)
    uint8_t hours;   ///< Hours (0-23)
    uint8_t day;     ///< Day of the week (1-7)
    uint8_t date;    ///< Date (1-31)
    uint8_t month;   ///< Month (1-12)
    uint8_t year;    ///< Year (0-99)
    bool century;    ///< Century flag (0-1)
} ds3231_data_t;

/**
 * @class DS3231
 * @brief Class for interfacing with the DS3231 real-time clock
 *
 * This class provides methods to set and get time from a DS3231 RTC module,
 * handle timezone offsets, perform clock synchronization, and more.
 */
class DS3231
{
public:
    /**
     * @brief Constructor for the DS3231 class
     *
     * @param[in] i2c_instance Pointer to the I2C instance to use
     */
    DS3231(i2c_inst_t *i2c_instance);
    /**
     * @brief Gets the singleton instance of the DS3231 class
     *
     * @return A reference to the singleton instance of the DS3231 class
     */
    static DS3231 &get_instance();

    /**
     * @brief Sets the RTC time using a Unix timestamp
     * @param unix_time Time in seconds since Unix epoch
     * @return 0 on success, -1 on failure
     */
    int set_time(time_t unix_time);

    /**
     * @brief Gets the current RTC time as Unix timestamp
     * @return Unix timestamp or -1 on error
     */
    time_t get_time();

    /**
     * @brief Reads the current temperature from the DS3231
     *
     * @param[out] resolution Pointer to store the temperature value in Celsius
     * @return 0 on success, -1 on failure
     */
    int read_temperature(float *resolution);

    /**
     * @brief Gets the current timezone offset
     *
     * @return Timezone offset in minutes (-720 to +720)
     */
    int16_t get_timezone_offset() const;

    /**
     * @brief Sets the timezone offset
     *
     * @param offset_minutes Offset in minutes (-720 to +720)
     */
    void set_timezone_offset(int16_t offset_minutes);

    /**
     * @brief Gets the current local time (including timezone offset)
     *
     * @return Unix timestamp adjusted for timezone, or -1 on error
     */
    time_t get_local_time();

private:
    i2c_inst_t *i2c;
    uint8_t ds3231_addr;
    recursive_mutex_t clock_mutex_;
    int16_t timezone_offset_minutes_ = 60;
    uint32_t sync_interval_minutes_ = 1440;
    time_t last_sync_time_ = 0;

    // Private constructor
    DS3231();

    // Delete copy constructor and assignment operator
    DS3231(const DS3231 &) = delete;
    DS3231 &operator=(const DS3231 &) = delete;

    /**
     * @brief Reads data from a specific register on the DS3231
     *
     * @param[in] reg_addr Register address to read from
     * @param[in] length Number of bytes to read
     * @param[out] data Buffer to store read data
     * @return 0 on success, -1 on failure
     */
    int i2c_read_reg(uint8_t reg_addr, size_t length, uint8_t *data);

    /**
     * @brief Writes data to a specific register on the DS3231
     *
     * @param[in] reg_addr Register address to write to
     * @param[in] length Number of bytes to write
     * @param[in] data Buffer containing data to write
     * @return 0 on success, -1 on failure
     */
    int i2c_write_reg(uint8_t reg_addr, size_t length, uint8_t *data);
};

#endif // DS3231_H