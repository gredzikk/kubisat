// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string>
#include <vector>
#include "protocol.h"
#include "event_manager.h"

bool initialize_radio();
void lora_tx_done_callback();
void on_receive(int packetSize);
void handle_uart_input();
void send_message(std::string outgoing);
void send_frame_uart(const Frame &frame);
void send_frame_lora(const Frame &frame);

std::vector<Frame> execute_command(uint32_t commandKey, const std::string &param, OperationType operationType);

void frame_process(const std::string &data, Interface interface);
std::string frame_encode(const Frame &frame);
Frame frame_decode(const std::string &data);
Frame frame_build(OperationType operation, uint8_t group, uint8_t command, const std::string &value, const ValueUnit unitType = ValueUnit::UNDEFINED);

#endif