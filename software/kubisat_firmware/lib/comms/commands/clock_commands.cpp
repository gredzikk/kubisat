// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "communication.h"
#include <time.h>
#include "DS3231.h"

static constexpr uint8_t clock_commands_group_id = 3;
static constexpr uint8_t time_command_id = 0;
static constexpr uint8_t timezone_offset_command_id = 1;
static constexpr uint8_t internal_temperature_command_id = 4;
/**
 * @defgroup ClockCommands Clock Management Commands
 * @brief Commands for managing system time and clock settings
 * @{
 */

/**
 * @brief Handler for getting and setting system time
 * @param param For SET: Unix timestamp as string, for GET: empty string
 * @param operationType GET/SET
 * @return Vector of frames containing success/error and current time or confirmation
 * @details
 * @note GET: <b>KBST;0;GET;3;0;;KBST</b>
 * @note When getting time, returns Unix timestamp
 * @note SET: <b>KBST;0;SET;3;0;TIMESTAMP;KBST</b>
 * @note When setting time, expects Unix timestamp as parameter
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.0
 */
std::vector<Frame> handle_time(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType == OperationType::SET)
    {
        if (param.empty())
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_REQUIRED);
            frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
            return frames;
        }
        try
        {
            time_t newTime = std::stoll(param);
            if (newTime <= 1742487032 || newTime >= 1893520044)
            {
                error_msg = error_code_to_string(ErrorCode::INVALID_VALUE);
                frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
                return frames;
            }

            if (DS3231::get_instance().set_time(newTime) != 0)
            {
                error_msg = error_code_to_string(ErrorCode::FAIL_TO_SET);
                frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
                return frames;
            }

            EventEmitter::emit(EventGroup::CLOCK, ClockEvent::CHANGED);
            frames.push_back(frame_build(OperationType::RES, clock_commands_group_id, time_command_id, std::to_string(DS3231::get_instance().get_time())));
            return frames;
        }
        catch (...)
        {
            error_msg = error_code_to_string(ErrorCode::INVALID_FORMAT);
            frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
            return frames;
        }
    }
    else if (operationType == OperationType::GET)
    {
        if (!param.empty())
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
            frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
            return frames;
        }

        uint32_t time_unix = DS3231::get_instance().get_local_time();
        if (time_unix == 0)
        {
            error_msg = error_code_to_string(ErrorCode::INTERNAL_FAIL_TO_READ);
            frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
            return frames;
        }

        frames.push_back(frame_build(OperationType::VAL, clock_commands_group_id, time_command_id, std::to_string(time_unix)));
        return frames;
    }

    error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
    frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, time_command_id, error_msg));
    return frames;
}

/**
 * @brief Handler for getting and setting timezone offset
 * @param param For SET: Timezone offset in minutes (-720 to +720), for GET: empty string
 * @param operationType GET/SET
 * @return Vector of frames containing success/error and timezone offset in minutes
 * @note GET: <b>KBST;0;GET;3;1;;KBST</b>
 * @note SET: <b>KBST;0;SET;3;1;OFFSET;KBST</b>
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.1
 */
std::vector<Frame> handle_timezone_offset(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (!(operationType == OperationType::GET || operationType == OperationType::SET))
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, timezone_offset_command_id, error_msg));
        return frames;
    }

    if (operationType == OperationType::GET)
    {
        if (!param.empty())
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_UNNECESSARY);
            frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, timezone_offset_command_id, error_msg));
            return frames;
        }

        int offset = DS3231::get_instance().get_timezone_offset();
        std::string offset_set = std::to_string(offset);
        frames.push_back(frame_build(OperationType::VAL, clock_commands_group_id, timezone_offset_command_id, offset_set));
        return frames;
    }

    if (param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_REQUIRED);
        frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, timezone_offset_command_id, error_msg));
        return frames;
    }

    try
    {
        int16_t offset = std::stoi(param);
        if (offset < -720 || offset > 720)
        {
            error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
            frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, timezone_offset_command_id, error_msg));
            return frames;
        }

        DS3231::get_instance().set_timezone_offset(offset);
        std::string offset_set = std::to_string(offset);
        frames.push_back(frame_build(OperationType::RES, clock_commands_group_id, timezone_offset_command_id, offset_set));
        return frames;
    }
    catch (...)
    {
        error_msg = error_code_to_string(ErrorCode::PARAM_INVALID);
        frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, timezone_offset_command_id, error_msg));
        return frames;
    }
}

/**
 * @brief Handler for reading the DS3231's internal temperature sensor
 * @param param Empty string expected
 * @param operationType GET
 * @return Vector with frame containing success/error and temperature in Celsius
 * @note GET: <b>KBST;0;GET;3;4;;KBST</b>
 * @note Returns temperature in format "XX.XX" where XX.XX is temperature in Celsius
 * @ingroup ClockCommands
 * @xrefitem command "Command" "Clock Commands" Command ID: 3.4
 */
std::vector<Frame> handle_get_internal_temperature(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    std::string error_msg;

    if (operationType != OperationType::GET || !param.empty())
    {
        error_msg = error_code_to_string(ErrorCode::INVALID_OPERATION);
        frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, internal_temperature_command_id, error_msg));
        return frames;
    }

    float temperature;
    if (DS3231::get_instance().read_temperature(&temperature) != 0)
    {
        error_msg = error_code_to_string(ErrorCode::INTERNAL_FAIL_TO_READ);
        frames.push_back(frame_build(OperationType::ERR, clock_commands_group_id, internal_temperature_command_id, error_msg));
        return frames;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << temperature;
    frames.push_back(frame_build(OperationType::VAL, clock_commands_group_id, internal_temperature_command_id, ss.str(), ValueUnit::CELSIUS));

    return frames;
}

/** @} */ // end of ClockCommands group