// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 *
 * @file system_state_manager.h
 *
 * @brief Manages the system state of the Kubisat firmware.
 *
 * @details This class is a singleton that provides methods for getting and setting
 *          various system states, such as whether a bootloader reset is pending,
 *          whether GPS collection is paused, whether the SD card is mounted, and
 *          the UART verbosity level.
 *
 * @defgroup SystemStateManager System State Manager
 * @brief Classes for handling system state management.
 *
 * @{
 */

#ifndef SYSTEM_STATE_MANAGER_H
#define SYSTEM_STATE_MANAGER_H

#include <mutex>
#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"

/**
 * @brief Enumeration of system operating modes.
 */
enum class SystemOperatingMode : uint8_t
{
    BATTERY_POWERED = 0, /**< Battery powered mode */
    USB_POWERED = 1      /**< USB powered mode */
};

/**
 * @brief Manages the system state of the Kubisat firmware.
 * @details This class is a singleton that provides methods for getting and setting
 *          various system states, such as whether a bootloader reset is pending,
 *          whether GPS collection is paused, whether the SD card is mounted, and
 *          the UART verbosity level.
 * @ingroup SystemStateManager
 */
class SystemStateManager
{
private:
    /** @brief Flag indicating whether a bootloader reset is pending. */
    bool pending_bootloader_reset;
    /** @brief Flag indicating whether GPS collection is paused. */
    bool gps_collection_paused;
    /** @brief Flag indicating whether the SD card is mounted. */
    bool sd_card_mounted;
    /** @brief The UART verbosity level. */
    VerbosityLevel uart_verbosity;
    /** @brief Flag indicating whether the SD card initialization was successful. */
    bool sd_card_init_status;
    /** @brief Flag indicating whether the radio initialization was successful. */
    bool radio_init_status;
    /** @brief Flag indicating whether the light sensor initialization was successful. */
    bool light_sensor_init_status;
    /** @brief Flag indicating whether the environment sensor initialization was successful. */
    bool env_sensor_init_status;
    /** @brief The system operating mode. */
    SystemOperatingMode system_operating_mode;
    /** @brief Mutex for thread-safe access to the system state. */
    recursive_mutex_t mutex_;

    /**
     * @brief Private constructor for the singleton pattern.
     * @details Initializes the system state and mutex.
     */
    SystemStateManager() : pending_bootloader_reset(false),
                           gps_collection_paused(false),
                           sd_card_mounted(false),
                           uart_verbosity(VerbosityLevel::DEBUG),
                           sd_card_init_status(false),
                           radio_init_status(false),
                           light_sensor_init_status(false),
                           env_sensor_init_status(false),
                           system_operating_mode(SystemOperatingMode::BATTERY_POWERED)
    {
        recursive_mutex_init(&mutex_);
    }

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    SystemStateManager(const SystemStateManager &) = delete;
    /**
     * @brief Deleted assignment operator to prevent assignment.
     */
    SystemStateManager &operator=(const SystemStateManager &) = delete;

public:
    /**
     * @brief Gets the singleton instance of the SystemStateManager class.
     * @return A reference to the singleton instance.
     */
    static SystemStateManager &get_instance()
    {
        static SystemStateManager instance;
        return instance;
    }

    /**
     * @brief Checks if a bootloader reset is pending.
     * @return True if a bootloader reset is pending, false otherwise.
     */
    bool is_bootloader_reset_pending() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        bool result = pending_bootloader_reset;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets whether a bootloader reset is pending.
     * @param[in] pending True if a bootloader reset is pending, false otherwise.
     */
    void set_bootloader_reset_pending(bool pending)
    {
        recursive_mutex_enter_blocking(&mutex_);
        pending_bootloader_reset = pending;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Checks if GPS collection is paused.
     * @return True if GPS collection is paused, false otherwise.
     */
    bool is_gps_collection_paused() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        bool result = gps_collection_paused;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets whether GPS collection is paused.
     * @param[in] paused True if GPS collection is paused, false otherwise.
     */
    void set_gps_collection_paused(bool paused)
    {
        recursive_mutex_enter_blocking(&mutex_);
        gps_collection_paused = paused;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Checks if the SD card is mounted.
     * @return True if the SD card is mounted, false otherwise.
     */
    bool is_sd_card_mounted() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        bool result = sd_card_mounted;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets whether the SD card is mounted.
     * @param[in] mounted True if the SD card is mounted, false otherwise.
     */
    void set_sd_card_mounted(bool mounted)
    {
        recursive_mutex_enter_blocking(&mutex_);
        sd_card_mounted = mounted;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Gets the UART verbosity level.
     * @return The UART verbosity level.
     */
    VerbosityLevel get_uart_verbosity() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        VerbosityLevel result = uart_verbosity;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets the UART verbosity level.
     * @param[in] level The UART verbosity level.
     */
    void set_uart_verbosity(VerbosityLevel level)
    {
        recursive_mutex_enter_blocking(&mutex_);
        uart_verbosity = level;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Checks if the radio initialization was successful.
     * @return True if the radio initialization was successful, false otherwise.
     */
    bool is_radio_init_ok() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        bool result = radio_init_status;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets whether the radio initialization was successful.
     * @param[in] status True if the radio initialization was successful, false otherwise.
     */
    void set_radio_init_ok(bool status)
    {
        recursive_mutex_enter_blocking(&mutex_);
        radio_init_status = status;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Checks if the light sensor initialization was successful.
     * @return True if the light sensor initialization was successful, false otherwise.
     */
    bool is_light_sensor_init_ok() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        bool result = light_sensor_init_status;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets whether the light sensor initialization was successful.
     * @param[in] status True if the light sensor initialization was successful, false otherwise.
     */
    void set_light_sensor_init_ok(bool status)
    {
        recursive_mutex_enter_blocking(&mutex_);
        light_sensor_init_status = status;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Checks if the environment sensor initialization was successful.
     * @return True if the environment sensor initialization was successful, false otherwise.
     */
    bool is_env_sensor_init_ok() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        bool result = env_sensor_init_status;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets whether the environment sensor initialization was successful.
     * @param[in] status True if the environment sensor initialization was successful, false otherwise.
     */
    void set_env_sensor_init_ok(bool status)
    {
        recursive_mutex_enter_blocking(&mutex_);
        env_sensor_init_status = status;
        recursive_mutex_exit(&mutex_);
    }

    /**
     * @brief Gets the system operating mode.
     * @return The system operating mode.
     */
    SystemOperatingMode get_operating_mode() const
    {
        recursive_mutex_enter_blocking(const_cast<recursive_mutex_t *>(&mutex_));
        SystemOperatingMode result = system_operating_mode;
        recursive_mutex_exit(const_cast<recursive_mutex_t *>(&mutex_));
        return result;
    }

    /**
     * @brief Sets the system operating mode.
     * @param[in] mode The system operating mode.
     */
    void set_operating_mode(SystemOperatingMode mode)
    {
        recursive_mutex_enter_blocking(&mutex_);
        system_operating_mode = mode;
        recursive_mutex_exit(&mutex_);
    }
};

#endif
/** @} */