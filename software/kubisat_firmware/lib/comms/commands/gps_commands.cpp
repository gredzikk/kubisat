// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "lib/location/gps_collector.h"
#include <sstream>
#include "system_state_manager.h"
#include "utils_parse.h"

static constexpr uint8_t gps_commands_group_id = 7;
static constexpr uint8_t power_status_command_id = 1;
static constexpr uint8_t passthrough_command_id = 2;

/**
 * @defgroup GPSCommands GPS Commands
 * @brief Commands for controlling and monitoring the GPS module
 * @{
 */

/**
 * @brief Handler for controlling GPS module power state
 * @param param For SET: "0" to power off, "1" to power on. For GET: empty
 * @param operationType GET to read current state, SET to change state
 * @return Vector of Frames containing:
 *         - Success: Current power state (0/1)
 *          or
 *         - Error: Error reason
 * @note <b>KBST;0;GET;7;1;;TSBK</b>
 * @note Return current GPS module power state: ON/OFF
 * @note <b>KBST;0;SET;7;1;POWER;TSBK</b>
 * @note POWER - 0 - OFF, 1 - ON
 * @ingroup GPSCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 7.1
 */
std::vector<Frame> handle_gps_power_status(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_str;

    if (operationType == OperationType::SET)
    {
        // command allowed only in ground mode
        SystemOperatingMode mode = SystemStateManager::get_instance().get_operating_mode();
        if (mode == SystemOperatingMode::BATTERY_POWERED)
        {
            error_str = error_code_to_string(ErrorCode::INVALID_OPERATION);
            frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, power_status_command_id, error_str));
            return frames;
        }

        if (param.empty())
        {
            error_str = error_code_to_string(ErrorCode::PARAM_REQUIRED);
            frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, power_status_command_id, error_str));
            return frames;
        }

        int32_t power_status = 0;
        if (!parse_int32(param, power_status) || (power_status != 0 && power_status != 1))
        {
            error_str = error_code_to_string(ErrorCode::PARAM_INVALID);
            frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, power_status_command_id, error_str));
            return frames;
        }
        gpio_put(GPS_POWER_ENABLE_PIN, power_status);
        EventEmitter::emit(EventGroup::GPS, power_status ? GPSEvent::POWER_ON : GPSEvent::POWER_OFF);
        frames.push_back(frame_build(OperationType::RES, gps_commands_group_id, power_status_command_id, std::to_string(power_status)));
        return frames;
    }
    else if (operationType == OperationType::GET)
    {
        if (!param.empty())
        {
            error_str = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
            frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, power_status_command_id, error_str));
            return frames;
        }

        bool power_status = gpio_get(GPS_POWER_ENABLE_PIN);
        frames.push_back(frame_build(OperationType::VAL, gps_commands_group_id, power_status_command_id, std::to_string(power_status)));
        return frames;
    }

    error_str = error_code_to_string(ErrorCode::INVALID_OPERATION);
    frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, power_status_command_id, error_str));
    return frames;
}

/**
 * @brief Handler for enabling GPS transparent mode (UART pass-through)
 * @param param TIMEOUT in seconds (optional, defaults to 60)
 * @param operationType SET
 * @return Vector of Frames containing:
 *         - Success: Exit message + reason
 *          or
 *         - Error: Error reason
 * @note <b>KBST;0;SET;7;2;TIMEOUT;TSBK</b>
 * @note TIMEOUT - 1-600s, default 60s
 * @note Enters a pass-through mode where UART communication is bridged directly to GPS
 * @note Send "##EXIT##" to exit mode before TIMEOUT
 * @ingroup GPSCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 7.2
 */
std::vector<Frame> handle_enable_gps_uart_passthrough(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_str;

    if (!(operationType == OperationType::SET))
    {
        error_str = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, passthrough_command_id, error_str));
        return frames;
    }

    // disable command if in battery mode
    SystemOperatingMode mode = SystemStateManager::get_instance().get_operating_mode();
    if (mode == SystemOperatingMode::BATTERY_POWERED)
    {
        error_str = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, passthrough_command_id, error_str));
        return frames;
    }

    uint32_t timeout_ms = 60000u;
    if (!param.empty())
    {
        int32_t seconds = 0;
        if (!parse_int32(param, seconds) || seconds < 1 || seconds > 600)
        {
            error_str = error_code_to_string(ErrorCode::INVALID_VALUE);
            frames.push_back(frame_build(OperationType::ERR, gps_commands_group_id, passthrough_command_id, error_str));
            return frames;
        }
        timeout_ms = static_cast<uint32_t>(seconds) * 1000u;
    }

    // Setup UART parameters and exit sequence
    const std::string EXIT_SEQUENCE = "##EXIT##";
    std::string input_buffer;
    bool exit_requested = false;
    SystemStateManager::get_instance().set_gps_collection_paused(true);
    sleep_ms(100);

    uint32_t original_baud_rate = DEBUG_UART_BAUD_RATE;
    uint32_t gps_baud_rate = GPS_UART_BAUD_RATE;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());

    EventEmitter::emit(EventGroup::GPS, GPSEvent::PASS_THROUGH_START);

    std::string message = "Entering GPS Serial Pass-Through Mode @" +
                          std::to_string(gps_baud_rate) + " for " +
                          std::to_string(timeout_ms / 1000) + "s\r\n" +
                          "Send " + EXIT_SEQUENCE + " to exit";
    uart_print(message, VerbosityLevel::INFO);

    sleep_ms(10);

    // Change main UART baudrate to GPS module baudrate for passthrough duration
    uart_set_baudrate(DEBUG_UART_PORT, gps_baud_rate);

    while (!exit_requested)
    {
        feed_watchdog();
        while (uart_is_readable(DEBUG_UART_PORT))
        {
            char ch = uart_getc(DEBUG_UART_PORT);

            input_buffer += ch;
            if (input_buffer.length() > EXIT_SEQUENCE.length())
            {
                input_buffer = input_buffer.substr(1);
            }

            if (input_buffer == EXIT_SEQUENCE)
            {
                exit_requested = true;
                break;
            }

            if (input_buffer != EXIT_SEQUENCE.substr(0, input_buffer.length()))
            {
                uart_write_blocking(GPS_UART_PORT,
                                    reinterpret_cast<const uint8_t *>(&ch), 1);
            }
        }

        while (uart_is_readable(GPS_UART_PORT))
        {
            char gps_byte = uart_getc(GPS_UART_PORT);
            uart_write_blocking(DEBUG_UART_PORT,
                                reinterpret_cast<const uint8_t *>(&gps_byte), 1);
        }

        if (to_ms_since_boot(get_absolute_time()) - start_time >= timeout_ms)
        {
            break;
        }
    }

    uart_set_baudrate(DEBUG_UART_PORT, original_baud_rate);

    sleep_ms(50);

    SystemStateManager::get_instance().set_gps_collection_paused(false);
    EventEmitter::emit(EventGroup::GPS, GPSEvent::PASS_THROUGH_END);

    std::string exit_reason = exit_requested ? "USER_EXIT" : "TIMEOUT";
    std::string response = "GPS UART BRIDGE EXIT: " + exit_reason;
    uart_print(response, VerbosityLevel::INFO);

    frames.push_back(frame_build(OperationType::RES, gps_commands_group_id, passthrough_command_id, response));
    return frames;
}
/** @} */ // end of GPSCommands group