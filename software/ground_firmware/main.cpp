// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief KubiSat Ground Station Firmware main implementation
 */

#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "stdio.h"
#include "string.h"
#include "string"
#include "utils.h"
#include "LoRa/LoRa-RP2040.h"

using std::string;

/**
 * @brief Address of this device (ground station)
 */
uint8_t localAddress = 21;

/**
 * @brief Address of the destination device (satellite)
 */
uint8_t destination = 37;

/**
 * @brief Timeout for waiting for a reply in milliseconds
 */
const uint32_t REPLY_TIMEOUT = 5000;

/**
 * @brief Special character to trigger RSSI/SNR append
 */
const char RSSI_SNR_TRIGGER = '?';

/**
 * @brief Sends a message via LoRa radio
 *
 * This function formats and transmits a message through the LoRa radio,
 * adding appropriate addressing and logging the transmission.
 *
 * @param outgoing The message payload to be sent
 */
void send_message(string outgoing)
{
    int n = outgoing.length();
    char send[n + 1];
    strcpy(send, outgoing.c_str());

    LoRa.beginPacket();       // start packet
    LoRa.write(destination);  // add destination address
    LoRa.write(localAddress); // add sender address
    LoRa.print(send);         // add payload
    LoRa.endPacket(false);    // finish packet and send it

    uart_print(send);

    LoRa.flush();
}


/**
 * @brief Initializes all system peripherals
 *
 * This function configures UART, initializes the LoRa radio module
 * with the specified frequency, and prepares the system for operation.
 */
void init_system()
{
    stdio_init_all();

    uart_init(DEBUG_UART_PORT, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(DEBUG_UART_PORT, false, false);
    uart_set_format(DEBUG_UART_PORT, 8, 1, UART_PARITY_NONE);

    uart_print("\nPower on");

    LoRa.setPins(csPin, resetPin, irqPin);
    long frequency = 433E6;
    if (!LoRa.begin(frequency))
    {
        uart_print("LoRa init failed. Check your connections.");
        while (true)
            ;
    }

    uart_print("LoRa initialized with frequency " + std::to_string(frequency));
}

/**
 * @brief Main program entry point
 *
 * This function implements the main control flow of the ground station firmware,
 * including the secure handshake protocol, connection maintenance, and
 * data forwarding between UART and LoRa interfaces.
 *
 * @return int Never returns (infinite loop)
 */
int main()
{
    init_system();
    uart_print("Starting LoRa <=> UART routine.");

    string uart_buffer;
    bool append_rssi_snr = false; 
    LoRa.receive();

    while (1)
    {
        int packet_size = LoRa.parsePacket();
        if (packet_size)
        {
            uint8_t buffer[256];
            int bytes_read = 0;
            while (LoRa.available() && bytes_read < packet_size && bytes_read < 256)
            {
                buffer[bytes_read++] = LoRa.read();
            }

            uint8_t receivedDestination = buffer[1];
            uint8_t receivedLocalAddress = buffer[0];

            if (receivedDestination != destination)
            {
                uart_print("Error: Destination address mismatch");
                LoRa.receive();
                continue;
            }

            if (receivedLocalAddress != localAddress)
            {
                uart_print("Error: Local address mismatch!");
                LoRa.receive();
                continue;
            }

            std::string received(reinterpret_cast<char *>(buffer + 2), bytes_read - 2);

            size_t header_pos = received.find("KBST");
            size_t footer_pos = received.find(";TSBK");

            if (header_pos != std::string::npos && footer_pos != std::string::npos && footer_pos > header_pos)
            {
                if (append_rssi_snr)
                {
                    std::string rssiSnr = ";" + std::to_string(LoRa.packetRssi()) + ";" + std::to_string(LoRa.packetSnr());
                    
                    char null_terminator = received.find('\0');
                    if (null_terminator != std::string::npos) {
                        received.erase(null_terminator);
                    }
                    
                    received += rssiSnr;

                    append_rssi_snr = false;
                }

                uart_print(received);
            }
            else
            {
                uart_print("No valid frame found in received data");
            }
            LoRa.receive();
        }

        while (uart_is_readable(DEBUG_UART_PORT))
        {
            char c = uart_getc(DEBUG_UART_PORT);
            if (c == '\n' || c == '\r')
            {
                if (!uart_buffer.empty())
                {
                    if (uart_buffer.find(RSSI_SNR_TRIGGER) != std::string::npos)
                    {
                        append_rssi_snr = true;
                        uart_buffer.erase(uart_buffer.find(RSSI_SNR_TRIGGER), 1); 
                        uart_print("RSSI/SNR append triggered");
                    }
                    send_message(uart_buffer);
                    uart_buffer.clear();
                }
            }
            else
            {
                uart_buffer += c;
            }
        }
        sleep_ms(10);
    }
}