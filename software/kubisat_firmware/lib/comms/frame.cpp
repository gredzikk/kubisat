// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "crc16.h"
#include "utils_parse.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>
#include <map>
#include <functional>
#include <cstdlib>

using CommandHandler = std::function<std::vector<Frame>(const std::string &, OperationType)>;
extern std::map<uint32_t, CommandHandler> command_handlers;

std::string frame_crc_payload(const Frame &frame)
{
    std::stringstream ss;
    ss << static_cast<int>(frame.direction) << DELIMITER
       << operation_type_to_string(frame.operationType) << DELIMITER
       << static_cast<int>(frame.group) << DELIMITER
       << static_cast<int>(frame.command) << DELIMITER
       << frame.value << DELIMITER
       << frame.unit << DELIMITER
       << frame.seq;
    return ss.str();
}

uint16_t frame_compute_crc(const Frame &frame)
{
    const std::string payload = frame_crc_payload(frame);
    return crc16_ccitt(reinterpret_cast<const uint8_t *>(payload.data()), payload.size());
}

void frame_assign_seq(Frame &frame, uint16_t seq)
{
    frame.seq = seq;
    frame.crc = frame_compute_crc(frame);
    frame.has_integrity = true;
}

std::string frame_encode(const Frame &frame)
{
    Frame encoded = frame;
    if (!encoded.has_integrity)
    {
        // Caller should assign seq; keep crc consistent if seq already set.
        encoded.crc = frame_compute_crc(encoded);
        encoded.has_integrity = true;
    }
    else
    {
        encoded.crc = frame_compute_crc(encoded);
    }

    std::ostringstream ss;
    ss << FRAME_BEGIN << DELIMITER
       << static_cast<int>(encoded.direction) << DELIMITER
       << operation_type_to_string(encoded.operationType) << DELIMITER
       << static_cast<int>(encoded.group) << DELIMITER
       << static_cast<int>(encoded.command) << DELIMITER
       << encoded.value << DELIMITER
       << encoded.unit << DELIMITER
       << std::dec << encoded.seq << DELIMITER;
    ss << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << encoded.crc;
    ss << std::dec << DELIMITER << FRAME_END;
    return ss.str();
}

static bool is_hex_u16(const std::string &token, uint16_t &out)
{
    if (token.empty() || token.size() > 4)
    {
        return false;
    }
    for (char c : token)
    {
        if (!std::isxdigit(static_cast<unsigned char>(c)))
        {
            return false;
        }
    }
    char *end = nullptr;
    unsigned long value = strtoul(token.c_str(), &end, 16);
    if (end == token.c_str() || *end != '\0' || value > 0xFFFF)
    {
        return false;
    }
    out = static_cast<uint16_t>(value);
    return true;
}

Frame frame_decode(const std::string &data)
{
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.footer = FRAME_END;

    std::stringstream ss(data);
    std::string token;

    uart_print("Decoding frame: " + data, VerbosityLevel::WARNING);

    if (!std::getline(ss, token, DELIMITER) || token != FRAME_BEGIN)
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_HEADER");
    }

    if (!std::getline(ss, token, DELIMITER))
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_DIR");
    }
    int32_t direction = 0;
    if (!parse_int32(token, direction) || (direction != 0 && direction != 1))
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_DIR");
    }
    frame.direction = static_cast<uint8_t>(direction);

    if (!std::getline(ss, token, DELIMITER))
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_MISSING_OP");
    }
    frame.operationType = string_to_operation_type(token);

    if (!std::getline(ss, token, DELIMITER))
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_MISSING_GROUP");
    }
    int32_t group = 0;
    if (!parse_int32(token, group) || group < 0 || group > 15)
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_GROUP");
    }
    frame.group = static_cast<uint8_t>(group);

    if (!std::getline(ss, token, DELIMITER))
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_MISSING_CMD");
    }
    int32_t command = 0;
    if (!parse_int32(token, command) || command < 0 || command > 32)
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_CMD");
    }
    frame.command = static_cast<uint8_t>(command);

    if (!std::getline(ss, token, DELIMITER))
    {
        frame.value = "";
    }
    else
    {
        frame.value = token;
    }

    // Remaining tokens: either legacy [unit] TSBK, or unit SEQ CRC TSBK, or SEQ CRC TSBK
    std::vector<std::string> rest;
    while (std::getline(ss, token, DELIMITER))
    {
        rest.push_back(token);
    }

    if (rest.empty())
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_FOOTER");
    }

    if (rest.back() != FRAME_END)
    {
        return frame_build(OperationType::ERR, 0, 0, "DECODE_INVALID_FOOTER");
    }
    rest.pop_back();

    if (rest.size() >= 2)
    {
        uint16_t crc_value = 0;
        uint16_t seq_value = 0;
        if (is_hex_u16(rest.back(), crc_value))
        {
            const std::string seq_token = rest[rest.size() - 2];
            if (parse_uint16(seq_token, seq_value))
            {
                frame.seq = seq_value;
                frame.crc = crc_value;
                if (rest.size() >= 3)
                {
                    frame.unit = rest[0];
                    for (size_t i = 1; i + 2 < rest.size(); ++i)
                    {
                        frame.unit += DELIMITER + rest[i];
                    }
                }
                else
                {
                    frame.unit = "";
                }

                const uint16_t expected = frame_compute_crc(frame);
                if (expected != frame.crc)
                {
                    uart_print("CRC mismatch", VerbosityLevel::ERROR);
                    return frame_build(OperationType::ERR, 0, 0, "DECODE_CRC_FAIL");
                }
                frame.has_integrity = true;
                return frame;
            }
        }
    }

    // Legacy frame without SEQ/CRC (UART transition compatibility)
    if (rest.size() == 1)
    {
        frame.unit = rest[0];
    }
    else if (rest.size() > 1)
    {
        frame.unit = rest[0];
    }
    frame.has_integrity = false;
    uart_print("Accepted legacy frame without SEQ/CRC", VerbosityLevel::DEBUG);
    return frame;
}

void frame_process(const std::string &data, Interface interface)
{
    gpio_put(PICO_DEFAULT_LED_PIN, false);

    Frame frame = frame_decode(data);
    if (frame.operationType == OperationType::ERR && frame.group == 0 && frame.command == 0 &&
        frame.value.rfind("DECODE_", 0) == 0)
    {
        if (interface == Interface::UART)
        {
            send_frame_uart(frame);
        }
        else
        {
            send_frame_lora(frame, false);
        }
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return;
    }

    if (frame.operationType == OperationType::ACK)
    {
        uint16_t ack_seq = frame.seq;
        if (!frame.has_integrity)
        {
            uint16_t parsed = 0;
            if (parse_uint16(frame.value, parsed))
            {
                ack_seq = parsed;
            }
        }
        notify_ack_received(ack_seq);
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return;
    }

    // Require integrity on LoRa path
    if (interface == Interface::LORA && !frame.has_integrity)
    {
        Frame error_frame = frame_build(OperationType::ERR, 0, 0, "CRC_REQUIRED");
        send_frame_lora(error_frame, false);
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        return;
    }

    if (interface == Interface::LORA && frame.has_integrity)
    {
        Frame ack = frame_build(OperationType::ACK, frame.group, frame.command, std::to_string(frame.seq));
        frame_assign_seq(ack, frame.seq);
        send_frame_lora(ack, false);
    }

    uint32_t command_key = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);
    std::vector<Frame> response_frames = execute_command(command_key, frame.value, frame.operationType);

    gpio_put(PICO_DEFAULT_LED_PIN, true);

    for (auto &response_frame : response_frames)
    {
        if (frame.has_integrity)
        {
            frame_assign_seq(response_frame, frame.seq);
        }
        if (interface == Interface::UART)
        {
            send_frame_uart(response_frame);
        }
        else if (interface == Interface::LORA)
        {
            // Command replies include SEQ/CRC; host retries on timeout. Sat retries unsolicited beacons.
            send_frame_lora(response_frame, false);
            sleep_ms(25);
        }
    }
}

Frame frame_build(OperationType operation, uint8_t group, uint8_t command,
                  const std::string &value, const ValueUnit unitType)
{
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.footer = FRAME_END;
    frame.direction = 1;
    frame.operationType = operation;
    frame.value = value;
    frame.unit = value_unit_type_to_string(unitType);
    frame.group = group;
    frame.command = command;
    frame.has_integrity = false;
    return frame;
}
