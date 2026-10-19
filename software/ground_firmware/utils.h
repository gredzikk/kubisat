// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "pin_config.h"

/** Timestamped log line for humans / debug. Do not use for protocol frames. */
void uart_print(const std::string &msg, uart_inst_t *uart = DEBUG_UART_PORT);

/** Raw line to the host (no timestamp). Use for KBST frames. */
void uart_write_raw(const std::string &msg, uart_inst_t *uart = DEBUG_UART_PORT);

#endif
