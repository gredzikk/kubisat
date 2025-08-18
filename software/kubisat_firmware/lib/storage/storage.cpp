// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file storage.cpp
 * @brief Implements file system operations for the Kubisat firmware.
 * @details This file contains functions for initializing the file system, opening, writing,
 *          reading, and closing files.
 *
 * @defgroup Storage Storage
 * @brief Classes and functions for managing file system operations.
 *
 * @{
 */

#include "storage.h"
#include "errno.h"
#include "utils.h"
#include "system_state_manager.h"

/**
 * @brief Initializes the file system on the SD card.
 * @return True if initialization was successful, false otherwise.
 * @details Mounts the littlefs file system on the SD card. If mounting fails, it formats the SD card
 *          with littlefs and then attempts to mount again.
 * @ingroup Storage
 */
bool fs_init(void)
{
    SystemStateManager::get_instance().set_sd_card_mounted(false);
    uart_print("fs_init littlefs on SD card", VerbosityLevel::DEBUG);
    blockdevice_t *sd = blockdevice_sd_create(SD_SPI_PORT,
                                              SD_MOSI_PIN,
                                              SD_MISO_PIN,
                                              SD_SCK_PIN,
                                              SD_CS_PIN,
                                              24 * MHZ,
                                              false);
    filesystem_t *fat = filesystem_fat_create();

    std::string status_string;
    int err = fs_mount("/", fat, sd);
    if (err == -1)
    {
        status_string = "Formatting / with FAT";
        uart_print(status_string, VerbosityLevel::WARNING);
        err = fs_format(fat, sd);
        if (err == -1)
        {
            status_string = "fs_format error: " + std::string(strerror(errno));
            uart_print(status_string, VerbosityLevel::ERROR);
            return false;
        }
        err = fs_mount("/", fat, sd);
        if (err == -1)
        {
            status_string = "fs_mount error: " + std::string(strerror(errno));
            uart_print(status_string, VerbosityLevel::ERROR);
            return false;
        }
    }

    SystemStateManager::get_instance().set_sd_card_mounted(true);
    return true;
}

/**
 * @brief Unmounts the file system from the SD card.
 * @return True if unmounting was successful, false otherwise.
 * @ingroup Storage
 */
bool fs_stop(void)
{
    int err = fs_unmount("/");
    if (err == -1)
    {
        uart_print("fs_unmount error", VerbosityLevel::ERROR);
        return false;
    }
    SystemStateManager::get_instance().set_sd_card_mounted(false);

    return true;
}
/** @} */