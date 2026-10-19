// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "LoRa-RP2040.h"
#include "pin_config.h"

#define MAX_PACKET_SIZE 255

/**
 * @brief Extract and process frames from a buffer
 * @param buffer The buffer containing potential frame data
 * @param interface The interface the data was received on (UART or LoRa)
 * @return bool True if at least one valid frame was found and processed
 * @ingroup ReceiveData
 */
bool extract_and_process_frames(const std::string &buffer, Interface interface)
{
    size_t search_pos = 0;
    bool found_frame = false;

    while (search_pos < buffer.length())
    {
        size_t header_pos = buffer.find(FRAME_BEGIN, search_pos);
        if (header_pos == std::string::npos)
            break;

        size_t footer_pos = buffer.find(FRAME_END, header_pos);
        if (footer_pos == std::string::npos)
            break;

        if (footer_pos > header_pos)
        {
            std::string frame_data = buffer.substr(header_pos, footer_pos + FRAME_END.length() - header_pos);
            uart_print("Extracted frame (length=" + std::to_string(frame_data.length()) +
                           "): " + frame_data,
                       VerbosityLevel::DEBUG);
            frame_process(frame_data, interface);
            found_frame = true;
        }

        search_pos = footer_pos + FRAME_END.length();
    }

    if (!found_frame)
    {
        uart_print("No valid frame found in received data", VerbosityLevel::WARNING);
    }

    return found_frame;
}

/**
 * @brief Process LoRa packet metadata and extract frame data
 * @param buffer The raw buffer containing the LoRa packet
 * @param bytes_read The number of bytes in the buffer
 * @return bool True if packet was valid and processed
 * @ingroup ReceiveData
 */
bool process_lora_packet(const std::vector<uint8_t> &buffer, int bytes_read)
{
    if (bytes_read < 2)
    {
        uart_print("Error: Packet too small to contain metadata!", VerbosityLevel::ERROR);
        return false;
    }

    uint8_t received_destination = buffer[0];
    uint8_t received_local_address = buffer[1];

    if (received_destination != lora_address_local)
    {
        uart_print("Error: Destination address mismatch!", VerbosityLevel::ERROR);
        return false;
    }

    if (received_local_address != lora_address_remote)
    {
        uart_print("Error: Local address mismatch!", VerbosityLevel::ERROR);
        return false;
    }

    // Skip 2 bytes being local and remote address appended by ground station
    int start_index = 2;
    std::string received(buffer.begin() + start_index, buffer.end());

    if (received.empty())
        return false;

    std::stringstream hex_dump;
    hex_dump << "Raw bytes: ";
    for (int i = 0; i < bytes_read; i++)
    {
        hex_dump << std::hex << std::setfill('0') << std::setw(2)
                 << static_cast<int>(buffer[i]) << " ";
    }
    uart_print(hex_dump.str(), VerbosityLevel::DEBUG);

    // Extract and process frames using the common function
    return extract_and_process_frames(received, Interface::LORA);
}

/**
 * @brief Callback function for handling received LoRa packets.
 * @param packet_size The size of the received packet.
 * @details Reads the received LoRa packet, extracts metadata, validates addresses,
 *          and processes any frames found in the data.
 * @ingroup ReceiveData
 */
void on_receive(int packet_size)
{
    if (packet_size == 0)
        return;
    uart_print("Received LoRa packet of size " + std::to_string(packet_size), VerbosityLevel::DEBUG);

    std::vector<uint8_t> buffer;
    buffer.reserve(packet_size);

    int bytes_read = 0;

    while (LoRa.available() && bytes_read < packet_size)
    {
        if (bytes_read >= MAX_PACKET_SIZE)
        {
            uart_print("Error: Packet exceeds maximum allowed size!", VerbosityLevel::ERROR);
            return;
        }
        buffer.push_back(LoRa.read());
        bytes_read++;
    }

    uart_print("Received " + std::to_string(bytes_read) + " bytes", VerbosityLevel::DEBUG);

    process_lora_packet(buffer, bytes_read);
}

/**
 * @brief Handles UART input.
 * @details Reads characters from the UART port, appends them to a buffer, and processes the buffer when a newline
 *          character is received, looking for valid frames in the received data.
 * @ingroup ReceiveData
 */
void handle_uart_input()
{
    static std::string uart_buffer;

    while (uart_is_readable(DEBUG_UART_PORT))
    {
        char c = uart_getc(DEBUG_UART_PORT);

        if (c == '\r' || c == '\n')
        {
            if (!uart_buffer.empty())
            {
                uart_print("Received UART string: " + uart_buffer, VerbosityLevel::DEBUG);
                extract_and_process_frames(uart_buffer, Interface::UART);
                uart_buffer.clear();
            }
        }
        else
        {
            uart_buffer += c;
        }
    }
}