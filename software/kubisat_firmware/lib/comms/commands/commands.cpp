// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "commands.h"
#include "communication.h"

/**
 * @defgroup CommandSystem Command System
 * @brief Core command system implementation
 * @{
 */
#define CMD(group, cmd) ((static_cast<uint32_t>(group) << 8) | static_cast<uint32_t>(cmd))

/**
 * @typedef CommandHandler
 * @brief Function type for command handlers
 */
using CommandHandler = std::function<std::vector<Frame>(const std::string &, OperationType)>;

/**
 * @typedef CommandMap
 * @brief Map type for storing command handlers
 */
using CommandMap = std::map<uint32_t, CommandHandler>;

/**
 * @brief Global map of all command handlers
 * @details Maps command keys (group << 8 | command) to their handler functions
 */
CommandMap command_handlers = {
    {CMD(1, 0), handle_get_commands_list},     // Group 1, Command 0
    {CMD(1, 1), handle_get_build_version},     // Group 1, Command 1
    {CMD(1, 2), handle_get_power_mode},        // Group 1, Command 2
    {CMD(1, 3), handle_get_uptime},            // Group 1, Command 3
    {CMD(1, 8), handle_verbosity},             // Group 1, Command 8
    {CMD(1, 9), handle_enter_bootloader_mode}, // Group 1, Command 9

    {CMD(3, 0), handle_time},                     // Group 3, Command 0
    {CMD(3, 1), handle_timezone_offset},          // Group 3, Command 1
    {CMD(3, 4), handle_get_internal_temperature}, // Group 3, Command 4

    {CMD(5, 1), handle_get_last_events}, // Group 5, Command 1
    {CMD(5, 2), handle_get_event_count}, // Group 5, Command 2

    {CMD(7, 1), handle_gps_power_status},            // Group 7, Command 1
    {CMD(7, 2), handle_enable_gps_uart_passthrough}, // Group 7, Command 2

    {CMD(8, 2), handle_get_last_telemetry_record}, // Group 8, Command 2
    {CMD(8, 3), handle_get_last_sensor_record},    // Group 8, Command 3
};

/**
 * @brief Executes a command based on its key
 * @param commandKey Combined group and command ID (group << 8 | command)
 * @param param Command parameter string
 * @param operationType Operation type (GET/SET)
 * @return Frame Response frame containing execution result
 * @details Looks up the command handler in commandHandlers map and executes it
 */
std::vector<Frame> execute_command(uint32_t commandKey, const std::string &param, OperationType operationType)
{
    auto it = command_handlers.find(commandKey);
    if (it != command_handlers.end())
    {
        CommandHandler handler = it->second;
        return handler(param, operationType);
    }
    else
    {
        std::vector<Frame> frames;
        frames.push_back(frame_build(OperationType::ERR, 0, 0, "INVALID COMMAND"));
        return frames;
    }
}
/** @} */ // end of CommandSystem group