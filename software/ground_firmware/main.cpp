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
#include "bridge_frame.h"
#include "lora_defaults.h"

using std::string;

/** Send a line containing only this character to append RSSI/SNR to the next RX frame. */
constexpr char RSSI_SNR_TRIGGER = '?';

static volatile int g_rx_packet_size = 0;
static volatile bool g_rx_pending = false;

enum class LedMode : uint8_t { Idle, Tx, Rx, Fault };

static void status_led(LedMode mode)
{
    // Brief patterns on the onboard LED for field debugging.
    switch (mode)
    {
    case LedMode::Fault:
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        break;
    case LedMode::Tx:
    case LedMode::Rx:
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(15);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        break;
    case LedMode::Idle:
    default:
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        break;
    }
}

static void lora_tx_done_callback()
{
    LoRa.receive(0);
}

static void lora_rx_callback(int packet_size)
{
    g_rx_packet_size = packet_size;
    g_rx_pending = true;
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

    status_led(LedMode::Tx);
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
    LoRa.receive(0);
    return true;
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

    const uint8_t received_destination = buffer[0];
    const uint8_t received_sender = buffer[1];

    if (!kubisat_bridge::address_ok(received_destination, received_sender,
                                    lora_address_local, lora_address_remote))
    {
        if (received_destination != lora_address_local)
            uart_print("Error: destination address mismatch");
        else
            uart_print("Error: sender address mismatch");
        LoRa.receive(0);
        return;
    }

    string received(reinterpret_cast<char *>(buffer + LORA_ADDRESS_BYTES),
                    static_cast<size_t>(bytes_read - LORA_ADDRESS_BYTES));
    kubisat_bridge::strip_trailing_nuls(received);

    string frame;
    const int rssi = LoRa.packetRssi();
    const float snr = LoRa.packetSnr();
    if (kubisat_bridge::extract_frame(received, append_rssi_snr, rssi, snr, frame))
    {
        if (append_rssi_snr)
        {
            append_rssi_snr = false;
        }
        status_led(LedMode::Rx);
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

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

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

    watchdog_enable(8000, true);

    LoRa.setPins(csPin, resetPin, irqPin);
    if (!LoRa.begin(KUBISAT_LORA_FREQUENCY_HZ))
    {
        uart_print("LoRa init failed. Check SPI wiring (expect version 0x12). Rebooting via watchdog.");
        status_led(LedMode::Fault);
        while (true)
        {
            tight_loop_contents();
        }
    }

    LoRa.onTxDone(lora_tx_done_callback);
    LoRa.onReceive(lora_rx_callback);
    LoRa.receive(0);

    uart_print("LoRa initialized at " + std::to_string(KUBISAT_LORA_FREQUENCY_HZ) +
               " Hz sync=0x" + std::to_string(KUBISAT_LORA_SYNC_WORD));
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

        if (g_rx_pending)
        {
            g_rx_pending = false;
            const int packet_size = g_rx_packet_size;
            if (packet_size > 0)
            {
                handle_lora_packet(packet_size, append_rssi_snr);
            }
        }

        // Fallback poll in case IRQ was missed (preserves continuous RX — see parse_packet).
        const int polled = LoRa.parsePacket();
        if (polled)
        {
            handle_lora_packet(polled, append_rssi_snr);
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
