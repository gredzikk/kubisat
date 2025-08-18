// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file storage.h
 * @brief Header file for file system operations on the Kubisat firmware.
 *
 * @details This file defines functions for initializing, mounting, and
 *          unmounting the file system on the SD card.
 *
 * @defgroup Storage Storage
 * @brief Classes and functions for managing file system operations.
 *
 * @{
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <stdio.h>
#include <string.h>
#include <hardware/clocks.h>
#include <hardware/flash.h>
#include "blockdevice/flash.h"
#include "blockdevice/sd.h"
#include "filesystem/littlefs.h"
#include "filesystem/vfs.h"
#include "pin_config.h"
#include "lfs.h"
#include "filesystem/fat.h"

/**
 * @brief Initializes the file system on the SD card.
 * @return True if initialization was successful, false otherwise.
 * @details Mounts the FAT file system on the SD card. If mounting fails, it
 *          formats the SD card with FAT and then attempts to mount again.
 * @ingroup Storage
 */
bool fs_init(void);

/**
 * @brief Unmounts the file system from the SD card.
 * @return True if unmounting was successful, false otherwise.
 * @ingroup Storage
 */
bool fs_stop(void);

#endif
/** @} */