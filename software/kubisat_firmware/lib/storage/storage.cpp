// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "storage.h"
#include "errno.h"
#include "utils.h"
#include "system_state_manager.h"

static blockdevice_t *g_sd_device = nullptr;
static filesystem_t *g_fat_fs = nullptr;

bool fs_init(void)
{
    SystemStateManager::get_instance().set_sd_card_mounted(false);
    uart_print("fs_init FAT on SD card (mount only)", VerbosityLevel::DEBUG);

    if (g_sd_device == nullptr)
    {
        g_sd_device = blockdevice_sd_create(SD_SPI_PORT,
                                           SD_MOSI_PIN,
                                           SD_MISO_PIN,
                                           SD_SCK_PIN,
                                           SD_CS_PIN,
                                           24 * MHZ,
                                           false);
    }
    if (g_fat_fs == nullptr)
    {
        g_fat_fs = filesystem_fat_create();
    }

    int err = fs_mount("/", g_fat_fs, g_sd_device);
    if (err == -1)
    {
        uart_print(std::string("fs_mount failed (not formatting): ") + strerror(errno),
                   VerbosityLevel::ERROR);
        return false;
    }

    SystemStateManager::get_instance().set_sd_card_mounted(true);
    return true;
}

bool fs_format_sd(void)
{
    uart_print("Explicit SD format requested", VerbosityLevel::WARNING);

    if (SystemStateManager::get_instance().is_sd_card_mounted())
    {
        fs_stop();
    }

    if (g_sd_device == nullptr)
    {
        g_sd_device = blockdevice_sd_create(SD_SPI_PORT,
                                           SD_MOSI_PIN,
                                           SD_MISO_PIN,
                                           SD_SCK_PIN,
                                           SD_CS_PIN,
                                           24 * MHZ,
                                           false);
    }
    if (g_fat_fs == nullptr)
    {
        g_fat_fs = filesystem_fat_create();
    }

    int err = fs_format(g_fat_fs, g_sd_device);
    if (err == -1)
    {
        uart_print(std::string("fs_format error: ") + strerror(errno), VerbosityLevel::ERROR);
        return false;
    }

    err = fs_mount("/", g_fat_fs, g_sd_device);
    if (err == -1)
    {
        uart_print(std::string("fs_mount after format error: ") + strerror(errno), VerbosityLevel::ERROR);
        SystemStateManager::get_instance().set_sd_card_mounted(false);
        return false;
    }

    SystemStateManager::get_instance().set_sd_card_mounted(true);
    uart_print("SD formatted and mounted", VerbosityLevel::WARNING);
    return true;
}

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
