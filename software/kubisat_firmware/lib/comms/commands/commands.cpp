// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "commands.h"
#include "communication.h"

#define CMD(group, cmd) ((static_cast<uint32_t>(group) << 8) | static_cast<uint32_t>(cmd))

using CommandHandler = std::function<std::vector<Frame>(const std::string &, OperationType)>;
using CommandMap = std::map<uint32_t, CommandHandler>;

CommandMap command_handlers = {
    {CMD(1, 0), handle_get_commands_list},
    {CMD(1, 1), handle_get_build_version},
    {CMD(1, 2), handle_get_power_mode},
    {CMD(1, 3), handle_get_uptime},
    {CMD(1, 4), handle_get_health},
    {CMD(1, 5), handle_format_sd},
    {CMD(1, 8), handle_verbosity},
    {CMD(1, 9), handle_enter_bootloader_mode},

    {CMD(2, 0), handle_imu_status},
    {CMD(2, 1), handle_imu_accel},
    {CMD(2, 2), handle_imu_gyro},

    {CMD(3, 0), handle_time},
    {CMD(3, 1), handle_timezone_offset},
    {CMD(3, 4), handle_get_internal_temperature},

    {CMD(5, 1), handle_get_last_events},
    {CMD(5, 2), handle_get_event_count},

    {CMD(7, 1), handle_gps_power_status},
    {CMD(7, 2), handle_enable_gps_uart_passthrough},

    {CMD(8, 2), handle_get_last_telemetry_record},
    {CMD(8, 3), handle_get_last_sensor_record},
    {CMD(8, 4), handle_get_recent_telemetry},
};

std::vector<Frame> execute_command(uint32_t commandKey, const std::string &param, OperationType operationType)
{
    auto it = command_handlers.find(commandKey);
    if (it != command_handlers.end())
    {
        return it->second(param, operationType);
    }

    std::vector<Frame> frames;
    frames.push_back(frame_build(OperationType::ERR, 0, 0, "INVALID COMMAND"));
    return frames;
}
