// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"

/**
 * @file send.cpp
 * @brief Implements functions for sending data, including LoRa messages and Frames.
 */

/**
 * @brief Sends a message using LoRa.
 * @param outgoing The message to send.
 * @details Converts the outgoing string to a C-style string, adds destination and local addresses,
 *          and sends the message using LoRa. Prints a log message to the UART.
 */
void send_message(std::string outgoing)
{
    std::vector<char> send(outgoing.length() + 1);
    strcpy(send.data(), outgoing.c_str());

    uart_print("LoRa packet begin", VerbosityLevel::DEBUG);
    LoRa.beginPacket();              // start packet
    LoRa.write(lora_address_remote); // add destination address
    LoRa.write(lora_address_local);  // add sender address
    for (size_t i = 0; i < send.size(); i++)
    {
        LoRa.write(static_cast<uint8_t>(send[i])); // add payload byte by byte
    }
    LoRa.endPacket(false); // finish packet and send it, param - async

    uart_print("LoRa packet end", VerbosityLevel::DEBUG);

    std::string message_to_log = "Sent message of size " + std::to_string(send.size());
    message_to_log += " to 0x" + std::to_string(lora_address_remote);
    message_to_log += " containing: " + std::string(send.data());

    uart_print(message_to_log, VerbosityLevel::DEBUG);

    LoRa.flush();
}

void send_frame_lora(const Frame &frame)
{
    uart_print("Sending frame via LoRa", VerbosityLevel::DEBUG);
    std::string outgoing = frame_encode(frame);
    send_message(outgoing);
    uart_print("Frame sent via LoRa", VerbosityLevel::DEBUG);
}

// If level is 0 - SILENT it means no diagnostic output but frame communications should still work
void send_frame_uart(const Frame &frame)
{
    std::string encoded_frame = frame_encode(frame);
    uart_print(encoded_frame, VerbosityLevel::SILENT);
}
