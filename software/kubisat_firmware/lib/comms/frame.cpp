// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"

using CommandHandler = std::function<std::vector<Frame>(const std::string &, OperationType)>;
extern std::map<uint32_t, CommandHandler> command_handlers;

/**
 * @file frame.cpp
 * @brief Implements functions for encoding, decoding, building, and processing Frames.
 * @defgroup FrameHandling Frame Handling
 * @brief Functions for encoding, decoding and building communication frames.
 * @{
 */

/**
 * @brief Encodes a Frame instance into a string.
 * @param frame The Frame instance to encode.
 * @return The Frame encoded as a string.
 * @details The encoded string includes the frame direction, operation type, group, command, value, and unit,
 *          all delimited by the DELIMITER character. The string is encapsulated by FRAME_BEGIN and FRAME_END.
 * @code
 * Frame myFrame;
 * myFrame.header = FRAME_BEGIN;
 * myFrame.direction = 0;
 * myFrame.operationType = OperationType::GET;
 * myFrame.group = 1;
 * myFrame.command = 1;
 * myFrame.value = "";
 * myFrame.unit = "";
 * myFrame.footer = FRAME_END;
 *
 * std::string encoded = frame_encode(myFrame);
 * // encoded will be "KBST;0;GET;1;1;;TSBK"
 * @endcode
 * @ingroup FrameHandling
 */
std::string frame_encode(const Frame &frame)
{
    std::stringstream ss;
    ss << static_cast<int>(frame.direction) << DELIMITER
       << operation_type_to_string(frame.operationType) << DELIMITER
       << static_cast<int>(frame.group) << DELIMITER
       << static_cast<int>(frame.command) << DELIMITER
       << frame.value;

    if (!frame.unit.empty())
    {
        ss << DELIMITER << frame.unit;
    }

    return FRAME_BEGIN + DELIMITER + ss.str() + DELIMITER + FRAME_END;
}

/**
 * @brief Decodes a string into a Frame instance.
 * @param encodedFrame The string to decode.
 * @return The Frame instance decoded from the string.
 * @throws std::runtime_error if the frame is invalid.
 * @details The decoded string is expected to be in the format:
 *          FRAME_BEGIN;direction;operationType;group;command;value;unit;FRAME_END
 * @ingroup FrameHandling
 */
Frame frame_decode(const std::string &data)
{
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.footer = FRAME_END;
    try
    {
        std::stringstream ss(data);
        std::string token;

        uart_print("Decoding frame: " + data, VerbosityLevel::WARNING);

        if (!std::getline(ss, token, DELIMITER) || token != FRAME_BEGIN)
        {
            throw std::runtime_error("DECODE_INVALID_HEADER");
        }

        std::getline(ss, token, DELIMITER);
        int direction = std::stoi(token);
        if (direction != 0 && direction != 1)
        {
            throw std::runtime_error("DECODE_INVALID_DIR");
        }
        frame.direction = direction;

        if (!std::getline(ss, token, DELIMITER))
        {
            throw std::runtime_error("DECODE_MISSING_OP");
        }
        frame.operationType = string_to_operation_type(token);

        if (!std::getline(ss, token, DELIMITER))
        {
            throw std::runtime_error("DECODE_MISSING_GROUP");
        }
        int group = std::stoi(token);
        if (group < 0 || group > 10)
        {
            throw std::runtime_error("DECODE_INVALID_GROUP");
        }
        frame.group = group;

        if (!std::getline(ss, token, DELIMITER))
        {
            throw std::runtime_error("DECODE_MISSING_CMD");
        }
        int command = std::stoi(token);
        if (command < 0 || command > 10)
        {
            throw std::runtime_error("DECODE_INVALID_CMD");
        }
        frame.command = command;

        if (!std::getline(ss, token, DELIMITER))
            frame.value = "";
        else
            frame.value = token;

        if (!std::getline(ss, token, DELIMITER))
            frame.unit = "";
        else
            frame.unit = token;

        std::getline(ss, token, DELIMITER);
        if (token != FRAME_END)
        {
            throw std::runtime_error("DECODE_INVALID_FOOTER");
        }
        frame.footer = token;

        return frame;
    }
    catch (const std::exception &e)
    {
        uart_print("Frame decode error: " + std::string(e.what()), VerbosityLevel::ERROR);
        return frame_build(OperationType::ERR, 0, 0, e.what());
    }
}

/**
 * @brief Executes a command based on the command key and the parameter.
 * @param data The Frame data in string format.
 * @details Decodes the frame data, extracts the command key, and executes the corresponding command.
 *          Sends the response frame. If an error occurs, an error frame is built and sent.
 */
void frame_process(const std::string &data, Interface interface)
{
    gpio_put(PICO_DEFAULT_LED_PIN, false);

    try
    {
        Frame frame = frame_decode(data);
        uint32_t command_key = (static_cast<uint32_t>(frame.group) << 8) | static_cast<uint32_t>(frame.command);

        std::vector<Frame> response_frames = execute_command(command_key, frame.value, frame.operationType);

        gpio_put(PICO_DEFAULT_LED_PIN, true);

        // Send all responses through the same interface that received the command
        for (const auto &response_frame : response_frames)
        {
            if (interface == Interface::UART)
            {
                send_frame_uart(response_frame);
            }
            else if (interface == Interface::LORA)
            {
                send_frame_lora(response_frame);
                sleep_ms(25);
            }
        }
    }
    catch (const std::exception &e)
    {
        Frame error_frame = frame_build(OperationType::ERR, 0, 0, e.what());
        if (interface == Interface::UART)
        {
            send_frame_uart(error_frame);
        }
        else if (interface == Interface::LORA)
        {
            send_frame_lora(error_frame);
        }
    }
}

/**
 * @brief Builds a Frame instance based on the execution result, group, command, value, and unit.
 * @param result The execution result.
 * @param group The group ID.
 * @param command The command ID within the group.
 * @param value The payload value.
 * @param unit The unit of measurement for the payload value.
 * @return The Frame instance.
 * @ingroup FrameHandling
 */
Frame frame_build(OperationType operation, uint8_t group, uint8_t command,
                  const std::string &value, const ValueUnit unitType)
{
    Frame frame;
    frame.header = FRAME_BEGIN;
    frame.footer = FRAME_END;

    switch (operation)
    {
    case OperationType::VAL:
        frame.direction = 1;
        frame.operationType = OperationType::VAL;
        frame.value = value;
        frame.unit = value_unit_type_to_string(unitType);
        break;

    case OperationType::ERR:
        frame.direction = 1;
        frame.operationType = OperationType::ERR;
        frame.value = value;
        frame.unit = value_unit_type_to_string(ValueUnit::UNDEFINED);
        break;

    case OperationType::RES:
        frame.direction = 1;
        frame.operationType = OperationType::RES;
        frame.value = value;
        frame.unit = value_unit_type_to_string(unitType);
        break;

    case OperationType::SEQ:
        frame.direction = 1;
        frame.operationType = OperationType::SEQ;
        frame.value = value;
        frame.unit = value_unit_type_to_string(unitType);
        break;
    default:
        break;
    }

    frame.group = group;
    frame.command = command;

    return frame;
}

/** @} */ // end of FrameHandling group