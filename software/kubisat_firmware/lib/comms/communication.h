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
void send_frame_lora(const Frame &frame, bool require_ack = false);
void notify_ack_received(uint16_t seq);
bool wait_for_ack(uint16_t seq, uint32_t timeout_ms);
void maybe_send_health_beacon();
void feed_watchdog();
void init_watchdog_and_boot_reason();

std::vector<Frame> execute_command(uint32_t commandKey, const std::string &param, OperationType operationType);

void frame_process(const std::string &data, Interface interface);
std::string frame_encode(const Frame &frame);
Frame frame_decode(const std::string &data);
Frame frame_build(OperationType operation, uint8_t group, uint8_t command, const std::string &value, const ValueUnit unitType = ValueUnit::UNDEFINED);
void frame_assign_seq(Frame &frame, uint16_t seq);

#endif
