// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include "event_manager.h"
#include <sstream>
#include <iomanip>
#include "utils_parse.h"

/**
 * @defgroup EventCommands Event Commands
 * @brief Commands for accessing and managing system event logs
 * @{
 */

static constexpr uint8_t event_commands_group_id = 5;
static constexpr uint8_t last_events_command_id = 1;
static constexpr uint8_t event_count_command_id = 2;

/**
 * @brief Handler for retrieving last N events from the event log
 * @param param Number of events to retrieve (optional, default 10). If 0, all events are returned.
 * @param operationType GET
 * @return Frame containing:
 *         - Success: A sequence of frames, each containing up to 10 hex-encoded events.
 *           Each event is in the format IIIITTTTTTTTGGEE, separated by '-'.
 *           - IIII: Event ID (16-bit, 4 hex characters)
 *           - TTTTTTTT: Unix Timestamp (32-bit, 8 hex characters)
 *           - GG: Event Group (8-bit, 2 hex characters)
 *           - EE: Event Type (8-bit, 2 hex characters)
 *           The last frame in the sequence is a VAL frame with the message "SEQ_DONE".
 *         - Error: A single frame with an error message:
 *           - "INVALID OPERATION": If the operation type is not GET.
 *           - "INVALID COUNT": If the count is greater than EVENT_BUFFER_SIZE.
 *           - "INVALID PARAMETER": If the parameter is not a valid unsigned integer.
 * @note <b>KBST;0;GET;5;1;[N];TSBK</b> - Retrieves the last N events. If N is 0, retrieves all events.
 * @note Returns up to 10 most recent events per frame.
 * @ingroup EventCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 5.1
 */
std::vector<Frame> handle_get_last_events(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET)
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, event_commands_group_id, last_events_command_id, error_msg));
        return frames;
    }

    size_t count = 10; // Default number of events to return
    if (!param.empty())
    {
        int32_t parsed = 0;
        if (!parse_int32(param, parsed) || parsed < 0)
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
            frames.push_back(frame_build(OperationType::ERR, event_commands_group_id, last_events_command_id, error_msg));
            return frames;
        }
        count = static_cast<size_t>(parsed);
        if (count > EVENT_BUFFER_SIZE)
        {
            error_msg = error_code_to_string(ErrorCode::INVALID_VALUE);
            frames.push_back(frame_build(OperationType::ERR, event_commands_group_id, last_events_command_id, error_msg));
            return frames;
        }
    }

    auto &event_manager = EventManager::get_instance();
    size_t available = event_manager.get_event_count();
    size_t to_return = (count == 0) ? available : std::min(count, available);
    size_t event_index = available;

    while (to_return > 0)
    {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        size_t events_in_frame = 0;

        for (size_t i = 0; i < 10 && to_return > 0; ++i)
        {
            event_index--;
            EventLog event = event_manager.get_event(event_index);

            ss << std::setw(4) << event.id
               << std::setw(8) << event.timestamp
               << std::setw(2) << static_cast<int>(event.group)
               << std::setw(2) << static_cast<int>(event.event);

            if (to_return > 1)
                ss << "-";
            to_return--;
            events_in_frame++;
        }
        frames.push_back(frame_build(OperationType::SEQ, event_commands_group_id, last_events_command_id, ss.str()));
    }
    frames.push_back(frame_build(OperationType::VAL, event_commands_group_id, last_events_command_id, "SEQ_DONE"));
    return frames;
}

/**
 * @brief Handler for getting total number of events in the log
 * @param param Empty string expected
 * @param operationType GET
 * @return Frame containing:
 *         - Success: Number of events currently in the log
 *         - Error: "INVALID REQUEST"
 * @note <b>KBST;0;GET;5;2;;TSBK</b>
 * @note Returns the total number of events in the log
 * @ingroup EventCommands
 * @xrefitem command "Command" "List of Commands" Command ID: 5.2
 */
std::vector<Frame> handle_get_event_count(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET || !param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, event_commands_group_id, event_count_command_id, error_msg));
        return frames;
    }

    auto &event_manager = EventManager::get_instance();
    frames.push_back(frame_build(OperationType::VAL, event_commands_group_id, event_count_command_id,
                                 std::to_string(event_manager.get_event_count())));
    return frames;
}
/** @} */ // end of EventCommands group