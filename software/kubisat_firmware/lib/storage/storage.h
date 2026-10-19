// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

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
 * @brief Mounts the FAT file system on the SD card (never auto-formats).
 * @return True if mount succeeded.
 */
bool fs_init(void);

/**
 * @brief Explicitly formats the SD card as FAT and remounts. Lab use only.
 * @return True on success.
 */
bool fs_format_sd(void);

/**
 * @brief Unmounts the file system from the SD card.
 */
bool fs_stop(void);

#endif
