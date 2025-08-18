// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file PowerManager.h
 * @brief Header file for the PowerManager class, which manages power-related functions.
 *
 * @details This file defines the PowerManager class, a singleton that provides
 *          methods for reading voltage and current values, configuring the
 *          INA3221 power monitor, and checking power alerts.
 *
 * @defgroup PowerManagement Power Management
 * @brief Classes for handling power-related functions.
 *
 * @{
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "INA3221/INA3221.h"
#include <map>
#include <string>
#include <hardware/i2c.h>
#include "pico/stdlib.h"
#include "pico/mutex.h"

/**
 * @brief Manages power-related functions.
 * @details This class is a singleton that provides methods for reading voltage
 *          and current values, configuring the INA3221 power monitor, and
 *          checking power alerts.
 * @ingroup PowerManagement
 */
class PowerManager
{
public:
    /**
     * @brief Constructor for the PowerManager class.
     * @param[in] i2c I2C instance to use for communication with the INA3221.
     */
    PowerManager(i2c_inst_t *i2c);

    /**
     * @brief Gets the singleton instance of the PowerManager class.
     * @return A reference to the singleton instance.
     */
    static PowerManager &get_instance();

    /**
     * @brief Initializes the PowerManager.
     * @return True if initialization was successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Reads the manufacturer and die IDs from the INA3221.
     * @return A string containing the manufacturer and die IDs.
     */
    std::string read_device_ids();

    /**
     * @brief Gets the solar charging current.
     * @return The solar charging current in milliamperes.
     */
    float get_current_charge_solar();

    /**
     * @brief Gets the USB charging current.
     * @return The USB charging current in milliamperes.
     */
    float get_current_charge_usb();

    /**
     * @brief Gets the total charging current.
     * @return The total charging current in milliamperes.
     */
    float get_current_charge_total();

    /**
     * @brief Gets the current draw.
     * @return The current draw in milliamperes.
     */
    float get_current_draw();

    /**
     * @brief Gets the battery voltage.
     * @return The battery voltage in volts.
     */
    float get_voltage_battery();

    /**
     * @brief Gets the 5V voltage.
     * @return The 5V voltage in volts.
     */
    float get_voltage_5v();

    /**
     * @brief Gets solar voltage.
     * @return The solar voltage in volts.
     */
    float get_voltage_solar();

    /**
     * @brief Configures the INA3221.
     * @param[in] config A map of configuration parameters.
     */
    void configure(ina3221_op_mode_t op_mode, ina3221_avg_mode_t avg_mode);

    /** @brief Solar current threshold in milliamperes. */
    static constexpr float SOLAR_CURRENT_THRESHOLD = 50.0f; // mA
    /** @brief USB current threshold in milliamperes. */
    static constexpr float USB_CURRENT_THRESHOLD = 50.0f; // mA
    /** @brief Low voltage threshold in volts. */
    static constexpr float BATTERY_LOW_THRESHOLD = 2.8f; // V
    /** @brief Overcharge voltage threshold in volts. */
    static constexpr float BATTERY_FULL_THRESHOLD = 4.2f; // V

private:
    /** @brief INA3221 instance for power monitoring. */
    INA3221 ina3221_;
    /** @brief Flag indicating if the PowerManager is initialized. */
    bool initialized_;
    /** @brief Mutex for thread-safe access to the PowerManager. */
    recursive_mutex_t powerman_mutex_;

    /**
     * @brief Private constructor for the singleton pattern.
     * @details Initializes the INA3221 and mutex.
     */
    PowerManager();

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    PowerManager(const PowerManager &) = delete;
    /**
     * @brief Deleted assignment operator to prevent assignment.
     */
    PowerManager &operator=(const PowerManager &) = delete;
};

#endif // POWER_MANAGER_H
       /** @} */