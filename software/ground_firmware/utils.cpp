// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "utils.h"

void uart_print(const std::string msg, uart_inst_t* uart)
{
    uint32_t timestamp = to_ms_since_boot(get_absolute_time());
    std::string msgToSend = "[" + std::to_string(timestamp) + "ms] - " + msg;
    uart_puts(uart, msgToSend.c_str());
    uart_puts(uart, "\r\n");
}
