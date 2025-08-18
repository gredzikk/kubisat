// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "commands.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "system_state_manager.h"
/**
 * @defgroup DiagnosticCommands Diagnostic Commands
 * @{
 */

static constexpr uint8_t diagnostic_commands_group_id = 1;
static constexpr uint8_t commands_list_command_id = 0;
static constexpr uint8_t build_version_command_id = 1;
static constexpr uint8_t power_mode_command_id = 2;
static constexpr uint8_t uptime_command_id = 3;
static constexpr uint8_t verbosity_command_id = 8;
static constexpr uint8_t enter_bootloader_command_id = 9;

/**
 * @brief Handler for listing all available commands on UART
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector of response frames - start frame, sequence of elements, end frame
 * @note <b>KBST;0;GET;1;0;;TSBK</b>
 * @note Print all available commands on UART port
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 0
 */
std::vector<Frame> handle_get_commands_list(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (!(operationType == OperationType::GET))
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, commands_list_command_id, error_msg));
        return frames;
    }

    if (!param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, commands_list_command_id, error_msg));
        return frames;
    }

    std::string combined_command_details;
    for (const auto &entry : command_handlers)
    {
        uint32_t command_key = entry.first;
        uint8_t group = (command_key >> 8) & 0xFF;
        uint8_t command = command_key & 0xFF;

        std::string command_details = std::to_string(group) + "." + std::to_string(command);

        if (combined_command_details.length() + command_details.length() + 1 > 100)
        {
            frames.push_back(frame_build(OperationType::SEQ, diagnostic_commands_group_id, commands_list_command_id, combined_command_details));
            combined_command_details = "";
        }

        if (!combined_command_details.empty())
        {
            combined_command_details += "-";
        }
        combined_command_details += command_details;
    }

    if (!combined_command_details.empty())
    {
        frames.push_back(frame_build(OperationType::SEQ, diagnostic_commands_group_id, commands_list_command_id, combined_command_details));
    }

    frames.push_back(frame_build(OperationType::VAL, diagnostic_commands_group_id, commands_list_command_id, "SEQ_DONE"));
    return frames;
}

/**
 * @brief Get firmware build version
 * @param param Empty string expected
 * @param operationType GET
 * @return One-element vector with result frame
 * @note <b>KBST;0;GET;1;1;;TSBK</b>
 * @note Get the firmware build version
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1
 */
std::vector<Frame> handle_get_build_version(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (!(operationType == OperationType::GET))
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, build_version_command_id, error_msg));
        return frames;
    }

    if (!param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, build_version_command_id, error_msg));
        return frames;
    }

    frames.push_back(frame_build(OperationType::VAL, diagnostic_commands_group_id, build_version_command_id, std::to_string(BUILD_NUMBER)));
    return frames;
}

/**
 * @brief Get system uptime
 * @param param Empty string expected
 * @param operationType GET
 * @return One-element vector with result frame
 * @note <b>KBST;0;GET;1;3;;TSBK</b>
 * @note Get the system uptime in seconds
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1.3
 */
std::vector<Frame> handle_get_uptime(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (!(operationType == OperationType::GET))
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, uptime_command_id, error_msg));
        return frames;
    }

    if (!param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, uptime_command_id, error_msg));
        return frames;
    }

    uint32_t uptime = to_ms_since_boot(get_absolute_time()) / 1000;
    frames.push_back(frame_build(OperationType::VAL, diagnostic_commands_group_id, uptime_command_id, std::to_string(uptime)));
    return frames;
}

/**
 * @brief Get system power mode
 * @param param Empty string expected
 * @param operationType GET
 * @return One-element vector with result frame
 * @note <b>KBST;0;GET;1;2;;TSBK</b>
 * @note Get the system power mode (BATTERY or USB)
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1.2
 */
std::vector<Frame> handle_get_power_mode(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (!(operationType == OperationType::GET))
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, power_mode_command_id, error_msg));
        return frames;
    }

    if (!param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, power_mode_command_id, error_msg));
        return frames;
    }

    SystemOperatingMode mode = SystemStateManager::get_instance().get_operating_mode();
    std::string mode_str = (mode == SystemOperatingMode::BATTERY_POWERED) ? "BATTERY" : "USB";
    frames.push_back(frame_build(OperationType::VAL, diagnostic_commands_group_id, power_mode_command_id, mode_str));
    return frames;
}

/**
 * @brief Handles setting or getting the UART verbosity level.
 *
 * This function allows the user to either retrieve the current UART verbosity
 * level or set a new verbosity level.
 *
 * @param param The desired verbosity level (0-5) as a string.  If empty, the
 *              current level is returned.
 * @param operationType The operation type.  Must be GET to retrieve the current
 *                      level, or SET to set a new level.
 * @return Vector containing one frame indicating the result of the operation.
 *         - Success (GET): Frame containing the current verbosity level.
 *         - Success (SET): Frame with "LEVEL SET" message.
 *         - Error: Frame with error message (e.g., "INVALID LEVEL (0-5)", "INVALID FORMAT").
 *
 * @note <b>KBST;0;GET;1;8;;TSBK</b> - Gets the current verbosity level.
 * @note <b>KBST;0;SET;1;8;[level];TSBK</b> - Sets the verbosity level.
 * @note Example: <b>KBST;0;SET;1;8;2;TSBK</b> - Sets the verbosity level to 2.
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 1.8
 */
std::vector<Frame> handle_verbosity(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType == OperationType::GET)
    {
        if (!param.empty())
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
            frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, verbosity_command_id, error_msg));
            return frames;
        }

        VerbosityLevel current_level = SystemStateManager::get_instance().get_uart_verbosity();
        uart_print("GET_VERBOSITY_" + std::to_string(static_cast<int>(current_level)),
                   VerbosityLevel::INFO);
        frames.push_back(frame_build(OperationType::VAL, diagnostic_commands_group_id, verbosity_command_id,
                                     std::to_string(static_cast<int>(current_level))));
        return frames;
    }
    else if (operationType == OperationType::SET)
    {
        try
        {
            int level = std::stoi(param);
            if (level < 0 || level > 4)
            {
                error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
                frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, verbosity_command_id, error_msg));
                return frames;
            }
            SystemStateManager::get_instance().set_uart_verbosity(static_cast<VerbosityLevel>(level));
            uart_print("SET_VERBOSITY_" + std::to_string(level), VerbosityLevel::WARNING);
            frames.push_back(frame_build(OperationType::RES, diagnostic_commands_group_id, verbosity_command_id, "LEVEL SET"));
            return frames;
        }
        catch (...)
        {
            error_msg = error_code_to_string(ErrorCode::INVALID_FORMAT);
            frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, verbosity_command_id, error_msg));
            return frames;
        }
    }
    else
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, verbosity_command_id, error_msg));
        return frames;
    }
}

/**
 * @brief Reboot system to USB firmware loader
 * @param param Empty string expected
 * @param operationType Must be SET
 * @return Frame with operation result
 * @note <b>KBST;0;SET;1;9;;TSBK</b>
 * @note Reboot the system to USB firmware loader
 * @ingroup DiagnosticCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 2
 */
std::vector<Frame> handle_enter_bootloader_mode(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::SET)
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, enter_bootloader_command_id, error_msg));
        return frames;
    }

    SystemOperatingMode mode = SystemStateManager::get_instance().get_operating_mode();
    if (mode == SystemOperatingMode::BATTERY_POWERED)
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, enter_bootloader_command_id, error_msg));
        return frames;
    }

    if (param != "USB")
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
        frames.push_back(frame_build(OperationType::ERR, diagnostic_commands_group_id, enter_bootloader_command_id, error_msg));
        return frames;
    }

    frames.push_back(frame_build(OperationType::RES, diagnostic_commands_group_id, enter_bootloader_command_id, "REBOOT BOOTSEL"));

    SystemStateManager::get_instance().set_bootloader_reset_pending(true);

    return frames;
}

/** @} */