// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pin_config.h"
#include <vector>

/**
 * @file utils.h
 * @brief Utility functions and definitions for the Kubisat firmware
 * @details Contains UART logging, color definitions, and CRC calculations
 */

/** @brief ANSI escape codes for terminal color output */
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_RESET "\033[0m"

/**
 * @brief Verbosity levels for logging system
 */
enum class VerbosityLevel : uint8_t
{
    SILENT = 0,  /**< No output */
    ERROR = 1,   /**< Only critical errors */
    WARNING = 2, /**< Warnings and errors */
    INFO = 3,    /**< Normal operation information */
    DEBUG = 4    /**< Detailed debug information */
};

/**
 * @brief Prints a message to UART with timestamp and formatting
 * @param msg The message to print
 * @param level Message verbosity level
 * @param uart The UART port to use
 */
void uart_print(const std::string &msg,
                VerbosityLevel level,
                uart_inst_t *uart = DEBUG_UART_PORT);

#endif