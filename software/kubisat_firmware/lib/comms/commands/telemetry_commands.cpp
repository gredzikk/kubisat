// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "commands.h"
#include "communication.h"
#include "telemetry_manager.h"

static constexpr uint8_t telemetry_commands_group = 8;
static constexpr uint8_t last_telemetry_command_id = 2;
static constexpr uint8_t last_sensor_command_id = 3;

/**
 * @defgroup TelemetryBufferCommands Telemetry Buffer Commands
 * @brief Commands for interacting with the telemetry buffer.
 * @{
 */

/**
 * @brief Handles the get last record command.
 *
 * This function reads the last record from the telemetry buffer, base64 encodes it,
 * and sends the encoded data as a response.
 *
 * @param param Unused.
 * @param operationType The operation type (must be GET).
 * @return A vector of Frames indicating the result of the operation.
 *         - Success: Frame with base64 encoded telemetry data.
 *         - Error: Frame with error message (e.g., "No telemetry data available").
 *
 * @note <b>KBST;0;GET;8;2;;TSBK</b>
 * @note This command retrieves the last telemetry record from the buffer and sends it base64 encoded.
 * @ingroup TelemetryBufferCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 8.2
 */
std::vector<Frame> handle_get_last_telemetry_record([[maybe_unused]] const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET)
    {
        if (!(param.empty()))
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
            frames.push_back(frame_build(OperationType::ERR, telemetry_commands_group, last_telemetry_command_id, error_msg));
            return frames;
        }
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, telemetry_commands_group, last_telemetry_command_id, error_msg));
        return frames;
    }

    std::string csv_data = TelemetryManager::get_instance().get_last_telemetry_record_csv();
    sleep_ms(10);

    if (csv_data.empty())
    {
        error_msg = "NO_DATA";
        frames.push_back(frame_build(OperationType::ERR, telemetry_commands_group, last_sensor_command_id, error_msg));
        return frames;
    }
    frames.push_back(frame_build(OperationType::VAL, telemetry_commands_group, last_telemetry_command_id, csv_data));

    return frames;
}

/**
 * @brief Handles the get last sensor record command.
 *
 * This function retrieves the last sensor record from the telemetry manager,
 * and sends the data as a response.
 *
 * @param param Unused.
 * @param operationType The operation type (must be GET).
 * @return A vector of Frames indicating the result of the operation.
 */
std::vector<Frame> handle_get_last_sensor_record([[maybe_unused]] const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET)
    {
        if (!(param.empty()))
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
            frames.push_back(frame_build(OperationType::ERR, telemetry_commands_group, last_telemetry_command_id, error_msg));
            return frames;
        }
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, telemetry_commands_group, last_sensor_command_id, error_msg));
        return frames;
    }

    std::string csv_data = TelemetryManager::get_instance().get_last_sensor_record_csv();
    sleep_ms(10);

    if (csv_data.empty())
    {
        error_msg = "NO_DATA";
        frames.push_back(frame_build(OperationType::ERR, telemetry_commands_group, last_sensor_command_id, error_msg));
        return frames;
    }

    frames.push_back(frame_build(OperationType::VAL, telemetry_commands_group, last_sensor_command_id, csv_data));

    return frames;
}
/** @} */ // TelemetryBufferCommands