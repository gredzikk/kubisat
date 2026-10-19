// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file main.cpp
 * @brief KubiSat Ground Station Firmware — UART <-> LoRa bridge
 */

#include "stdlib.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/watchdog.h"
#include "stdio.h"
#include "string.h"
#include "string"
#include "utils.h"
#include "pin_config.h"
#include "LoRa-RP2040.h"
#include "frame_markers.h"

using std::string;

/** Send a line containing only this character to append RSSI/SNR to the next RX frame. */
constexpr char RSSI_SNR_TRIGGER = '?';

static void lora_tx_done_callback()
{
    LoRa.receive(0);
}

/**
 * @brief Sends a payload via LoRa with dest/sender addressing.
 * @return true if the packet was handed to the radio
 */
static bool send_message(const string &outgoing)
{
    if (outgoing.empty())
    {
        return false;
    }

    if (outgoing.length() > static_cast<size_t>(LORA_MAX_PAYLOAD_SIZE))
    {
        uart_print("Error: payload exceeds LoRa max (" +
                   std::to_string(LORA_MAX_PAYLOAD_SIZE) + " bytes)");
        return false;
    }

    if (!LoRa.beginPacket())
    {
        uart_print("Error: LoRa beginPacket failed (radio busy?)");
        return false;
    }

    LoRa.write(lora_address_remote); // destination (satellite)
    LoRa.write(lora_address_local);  // sender (ground)
    LoRa.write(reinterpret_cast<const uint8_t *>(outgoing.data()), outgoing.length());

    if (!LoRa.endPacket(false))
    {
        uart_print("Error: LoRa endPacket failed");
        LoRa.receive(0);
        return false;
    }

    uart_print("TX " + std::to_string(outgoing.length()) + " bytes");
    LoRa.flush();
    // onTxDone restores RX; also call receive in case IRQ is late/missed
    LoRa.receive(0);
    return true;
}

/**
 * @brief Extract first KBST…TSBK frame from payload; optionally append ;rssi;snr.
 */
static bool extract_frame(const string &payload, bool append_rssi_snr, string &out_frame)
{
    size_t header_pos = payload.find(FRAME_BEGIN);
    if (header_pos == string::npos)
    {
        return false;
    }

    size_t footer_pos = payload.find(FRAME_END, header_pos);
    if (footer_pos == string::npos || footer_pos < header_pos)
    {
        return false;
    }

    out_frame = payload.substr(header_pos, footer_pos + strlen(FRAME_END) - header_pos);

    if (append_rssi_snr)
    {
        out_frame += FRAME_DELIMITER;
        out_frame += std::to_string(LoRa.packetRssi());
        out_frame += FRAME_DELIMITER;
        out_frame += std::to_string(LoRa.packetSnr());
    }

    return true;
}

static void strip_trailing_nuls(string &s)
{
    while (!s.empty() && s.back() == '\0')
    {
        s.pop_back();
    }
}

static void handle_lora_packet(int packet_size, bool &append_rssi_snr)
{
    uint8_t buffer[LORA_MAX_PACKET_SIZE];
    int bytes_read = 0;
    while (LoRa.available() && bytes_read < packet_size && bytes_read < LORA_MAX_PACKET_SIZE)
    {
        buffer[bytes_read++] = static_cast<uint8_t>(LoRa.read());
    }

    if (bytes_read < LORA_ADDRESS_BYTES)
    {
        uart_print("Error: packet too small for addressing");
        LoRa.receive(0);
        return;
    }

    // Wire format: [destination, sender, payload…] — same as satellite
    const uint8_t received_destination = buffer[0];
    const uint8_t received_sender = buffer[1];

    if (received_destination != lora_address_local)
    {
        uart_print("Error: destination address mismatch");
        LoRa.receive(0);
        return;
    }

    if (received_sender != lora_address_remote)
    {
        uart_print("Error: sender address mismatch");
        LoRa.receive(0);
        return;
    }

    string received(reinterpret_cast<char *>(buffer + LORA_ADDRESS_BYTES),
                    static_cast<size_t>(bytes_read - LORA_ADDRESS_BYTES));
    strip_trailing_nuls(received);

    string frame;
    if (extract_frame(received, append_rssi_snr, frame))
    {
        if (append_rssi_snr)
        {
            append_rssi_snr = false;
        }
        uart_write_raw(frame);
    }
    else
    {
        uart_print("No valid frame found in received data");
    }

    LoRa.receive(0);
}

static void handle_uart_line(string &uart_buffer, bool &append_rssi_snr)
{
    if (uart_buffer.empty())
    {
        return;
    }

    // Standalone "?" arms RSSI/SNR append for the next RX frame (not transmitted).
    if (uart_buffer.size() == 1 && uart_buffer[0] == RSSI_SNR_TRIGGER)
    {
        append_rssi_snr = true;
        uart_print("RSSI/SNR append armed for next RX");
        uart_buffer.clear();
        return;
    }

    send_message(uart_buffer);
    uart_buffer.clear();
}

static void init_system()
{
    stdio_init_all();

    uart_init(DEBUG_UART_PORT, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(DEBUG_UART_PORT, false, false);
    uart_set_format(DEBUG_UART_PORT, 8, 1, UART_PARITY_NONE);

    if (watchdog_caused_reboot())
    {
        uart_print("Reboot: watchdog");
    }
    else
    {
        uart_print("Power on");
    }

    // ~8s; kicked in the main loop
    watchdog_enable(8000, true);

    LoRa.setPins(csPin, resetPin, irqPin);
    const long frequency = 433E6;
    if (!LoRa.begin(frequency))
    {
        uart_print("LoRa init failed. Check SPI wiring (expect version 0x12). Rebooting via watchdog.");
        // Do not kick the watchdog — let it reset the board.
        while (true)
        {
            tight_loop_contents();
        }
    }

    LoRa.onTxDone(lora_tx_done_callback);
    LoRa.receive(0);

    uart_print("LoRa initialized at " + std::to_string(frequency) + " Hz");
}

int main()
{
    init_system();
    uart_print("Starting LoRa <=> UART bridge.");

    string uart_buffer;
    uart_buffer.reserve(LORA_MAX_PAYLOAD_SIZE);
    bool append_rssi_snr = false;

    while (true)
    {
        watchdog_update();

        const int packet_size = LoRa.parsePacket();
        if (packet_size)
        {
            handle_lora_packet(packet_size, append_rssi_snr);
        }

        while (uart_is_readable(DEBUG_UART_PORT))
        {
            const char c = uart_getc(DEBUG_UART_PORT);
            if (c == '\n' || c == '\r')
            {
                handle_uart_line(uart_buffer, append_rssi_snr);
            }
            else if (uart_buffer.size() < static_cast<size_t>(LORA_MAX_PAYLOAD_SIZE))
            {
                uart_buffer += c;
            }
            else
            {
                uart_print("Error: UART line overflow, dropping buffer");
                uart_buffer.clear();
            }
        }

        sleep_ms(1);
    }
}
