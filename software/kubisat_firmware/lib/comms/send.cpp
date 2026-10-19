// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "LoRa-RP2040.h"
#include "system_state_manager.h"
#include "pico/time.h"
#include <cstring>
#include <vector>

static volatile uint16_t g_last_ack_seq = 0;
static volatile bool g_ack_pending = false;
static uint16_t g_next_tx_seq = 1;

static uint16_t allocate_seq()
{
    uint16_t seq = g_next_tx_seq++;
    if (g_next_tx_seq == 0)
    {
        g_next_tx_seq = 1;
    }
    return seq;
}

void notify_ack_received(uint16_t seq)
{
    g_last_ack_seq = seq;
    g_ack_pending = true;
}

bool wait_for_ack(uint16_t seq, uint32_t timeout_ms)
{
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (!time_reached(deadline))
    {
        int packet_size = LoRa.parse_packet();
        if (packet_size)
        {
            on_receive(packet_size);
        }
        if (g_ack_pending && g_last_ack_seq == seq)
        {
            g_ack_pending = false;
            return true;
        }
        feed_watchdog();
        sleep_ms(5);
    }
    return false;
}

void send_message(std::string outgoing)
{
    std::vector<char> send(outgoing.length() + 1);
    strcpy(send.data(), outgoing.c_str());

    uart_print("LoRa packet begin", VerbosityLevel::DEBUG);
    LoRa.beginPacket();
    LoRa.write(lora_address_remote);
    LoRa.write(lora_address_local);
    for (size_t i = 0; i < send.size(); i++)
    {
        LoRa.write(static_cast<uint8_t>(send[i]));
    }
    LoRa.endPacket(false);

    uart_print("Sent: " + outgoing, VerbosityLevel::DEBUG);
    LoRa.flush();
}

void send_frame_lora(const Frame &frame, bool require_ack)
{
    if (!SystemStateManager::get_instance().is_radio_init_ok())
    {
        uart_print("LoRa send skipped (radio not ready)", VerbosityLevel::WARNING);
        return;
    }

    constexpr int kMaxAttempts = 3;
    constexpr uint32_t kAckTimeoutMs = 750;

    Frame to_send = frame;
    if (!to_send.has_integrity)
    {
        frame_assign_seq(to_send, allocate_seq());
    }
    else
    {
        to_send.crc = frame_compute_crc(to_send);
    }

    for (int attempt = 0; attempt < (require_ack ? kMaxAttempts : 1); ++attempt)
    {
        g_ack_pending = false;
        std::string outgoing = frame_encode(to_send);
        send_message(outgoing);

        if (!require_ack)
        {
            return;
        }

        if (wait_for_ack(to_send.seq, kAckTimeoutMs))
        {
            return;
        }
        uart_print("ACK timeout seq " + std::to_string(to_send.seq), VerbosityLevel::WARNING);
        sleep_ms(100);
    }
}

void send_frame_uart(const Frame &frame)
{
    Frame to_send = frame;
    if (!to_send.has_integrity)
    {
        frame_assign_seq(to_send, allocate_seq());
    }
    uart_print(frame_encode(to_send), VerbosityLevel::SILENT);
}
