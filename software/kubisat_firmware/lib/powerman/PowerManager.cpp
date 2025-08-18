// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file PowerManager.cpp
 * @brief Implementation of the PowerManager class, which manages power-related functions.
 *
 * @details This file implements the PowerManager class, a singleton that provides
 *          methods for reading voltage and current values, configuring the
 *          INA3221 power monitor, and checking power alerts.
 *
 * @defgroup PowerManagement Power Management
 * @brief Classes for handling power-related functions.
 *
 * @{
 */

#include "PowerManager.h"
#include <iostream>
#include <sstream>
#include "pin_config.h"

/**
 * @brief Private constructor for the singleton pattern.
 * @details Initializes the INA3221 and mutex.
 * @ingroup PowerManagement
 */
PowerManager::PowerManager()
    : ina3221_(INA3221_ADDR40_GND, MAIN_I2C_PORT)
{
    recursive_mutex_init(&powerman_mutex_);
}

/**
 * @brief Gets the singleton instance of the PowerManager class.
 * @return A reference to the singleton instance.
 * @ingroup PowerManagement
 */
PowerManager &PowerManager::get_instance()
{
    static PowerManager instance;
    return instance;
}

/**
 * @brief Initializes the PowerManager.
 * @return True if initialization was successful, false otherwise.
 * @ingroup PowerManagement
 */
bool PowerManager::initialize()
{
    recursive_mutex_enter_blocking(&powerman_mutex_);
    initialized_ = ina3221_.begin();

    recursive_mutex_exit(&powerman_mutex_);
    return initialized_;
}

/**
 * @brief Reads the manufacturer and die IDs from the INA3221.
 * @return A string containing the manufacturer and die IDs.
 * @ingroup PowerManagement
 */
std::string PowerManager::read_device_ids()
{
    if (!initialized_)
        return "noinit";
    recursive_mutex_enter_blocking(&powerman_mutex_);
    std::stringstream man_ss;
    man_ss << std::hex << ina3221_.get_manufacturer_id();
    std::string MAN = "MAN 0x" + man_ss.str();

    std::stringstream die_ss;
    die_ss << std::hex << ina3221_.get_die_id();
    std::string DIE = "DIE 0x" + die_ss.str();
    recursive_mutex_exit(&powerman_mutex_);
    return MAN + " - " + DIE;
}

/**
 * @brief Gets the battery voltage.
 * @return The battery voltage in volts.
 * @ingroup PowerManagement
 */
float PowerManager::get_voltage_battery()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float voltage = ina3221_.get_voltage(INA3221_CH1);
    recursive_mutex_exit(&powerman_mutex_);
    return voltage;
}

/**
 * @brief Gets the 5V voltage.
 * @return The 5V voltage in volts.
 * @ingroup PowerManagement
 */
float PowerManager::get_voltage_5v()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float voltage = ina3221_.get_voltage(INA3221_CH2);
    recursive_mutex_exit(&powerman_mutex_);
    return voltage;
}

/**
 * @brief Gets the solar voltage.
 * @return The solar voltage in volts.
 * @ingroup PowerManagement
 */
float PowerManager::get_voltage_solar()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float voltage = ina3221_.get_voltage(INA3221_CH3);
    recursive_mutex_exit(&powerman_mutex_);
    return voltage;
}

/**
 * @brief Gets the USB charging current.
 * @return The USB charging current in milliamperes.
 * @ingroup PowerManagement
 */
float PowerManager::get_current_charge_usb()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH1);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

/**
 * @brief Gets the current draw.
 * @return The current draw in milliamperes.
 * @ingroup PowerManagement
 */
float PowerManager::get_current_draw()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH2);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

/**
 * @brief Gets the solar charging current.
 * @return The solar charging current in milliamperes.
 * @ingroup PowerManagement
 */
float PowerManager::get_current_charge_solar()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH3);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

/**
 * @brief Gets the total charging current.
 * @return The total charging current in milliamperes.
 * @ingroup PowerManagement
 */
float PowerManager::get_current_charge_total()
{
    if (!initialized_)
        return 0.0f;
    recursive_mutex_enter_blocking(&powerman_mutex_);
    float current = ina3221_.get_current_ma(INA3221_CH1) + ina3221_.get_current_ma(INA3221_CH3);
    recursive_mutex_exit(&powerman_mutex_);
    return current;
}

/**
 * @brief Configures the INA3221.
 * @param[in] op_mode Operating mode
 * @param[in] avg_mode Averaging mode.
 * @ingroup PowerManagement
 */
void PowerManager::configure(ina3221_op_mode_t op_mode, ina3221_avg_mode_t avg_mode)
{
    if (!initialized_)
        return;

    recursive_mutex_enter_blocking(&powerman_mutex_);

    if (op_mode == CONTINUOUS_MODE)
    {
        ina3221_.set_mode_continuous();
    }
    else
    {
        ina3221_.set_mode_triggered();
    }

    ina3221_.set_averaging_mode(avg_mode);

    recursive_mutex_exit(&powerman_mutex_);
}

/** @} */