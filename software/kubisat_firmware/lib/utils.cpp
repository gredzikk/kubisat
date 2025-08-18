// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "utils.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include <vector>
#include <queue>
#include <string>
#include <array>
#include "system_state_manager.h"

/**
 * @file utils.cpp
 * @brief Implementation of utility functions for the Kubisat firmware
 */

/** @brief Mutex for UART access protection */
namespace
{
    mutex_t uart_mutex;
}

/**
 * @brief Gets ANSI color code for verbosity level
 * @param level The verbosity level
 * @return ANSI color escape sequence
 */
std::string get_level_color(VerbosityLevel level)
{
    switch (level)
    {
    case VerbosityLevel::ERROR:
        return ANSI_RED;
    case VerbosityLevel::WARNING:
        return ANSI_YELLOW;
    case VerbosityLevel::INFO:
        return ANSI_GREEN;
    case VerbosityLevel::DEBUG:
        return ANSI_BLUE;
    default:
        return "";
    }
}

/**
 * @brief Gets text prefix for verbosity level
 * @param level The verbosity level
 * @return Text prefix for the level
 */
std::string get_level_prefix(VerbosityLevel level)
{
    switch (level)
    {
    case VerbosityLevel::ERROR:
        return "ERROR: ";
    case VerbosityLevel::WARNING:
        return "WARNING: ";
    case VerbosityLevel::INFO:
        return "INFO: ";
    case VerbosityLevel::DEBUG:
        return "DEBUG: ";
    default:
        return "";
    }
}

/**
 * @brief Prints a message to the UART with a timestamp and core number.
 * @param msg The message to print.
 * @param uart The UART instance to use for printing.
 * @details Prints the given message to the specified UART, prepending it with a timestamp and the core number.
 *          Uses a mutex to ensure thread-safe access to the UART.
 */
void uart_print(const std::string &msg, VerbosityLevel level, uart_inst_t *uart)
{
    if (static_cast<int>(level) > static_cast<int>(SystemStateManager::get_instance().get_uart_verbosity()))
    {
        return;
    }

    static bool mutex_inited = false;
    if (!mutex_inited)
    {
        mutex_init(&uart_mutex);
        mutex_inited = true;
    }

    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    uint core_num = get_core_num();

    std::string color = get_level_color(level);
    std::string prefix = get_level_prefix(level);
    std::string msg_to_send = "[" + std::to_string(timestamp) + "ms] - Core " +
                              std::to_string(core_num) + ": " +
                              color + prefix + ANSI_RESET + msg + "\r\n";

    mutex_enter_blocking(&uart_mutex);
    uart_puts(uart, msg_to_send.c_str());
    mutex_exit(&uart_mutex);
}
