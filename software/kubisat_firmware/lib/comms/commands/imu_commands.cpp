// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "commands.h"
#include "communication.h"
#include "ISensor.h"
#include "system_state_manager.h"
#include <sstream>
#include <iomanip>

static constexpr uint8_t imu_commands_group_id = 2;
static constexpr uint8_t imu_status_command_id = 0;
static constexpr uint8_t imu_accel_command_id = 1;
static constexpr uint8_t imu_gyro_command_id = 2;

std::vector<Frame> handle_imu_status(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    if (operationType != OperationType::GET || !param.empty())
    {
        frames.push_back(frame_build(OperationType::ERR, imu_commands_group_id, imu_status_command_id,
                                     error_code_to_string(ErrorCode::INVALID_OPERATION)));
        return frames;
    }
    bool ok = SystemStateManager::get_instance().is_imu_sensor_init_ok();
    frames.push_back(frame_build(OperationType::VAL, imu_commands_group_id, imu_status_command_id, ok ? "OK" : "FAIL"));
    return frames;
}

std::vector<Frame> handle_imu_accel(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    if (operationType != OperationType::GET || !param.empty())
    {
        frames.push_back(frame_build(OperationType::ERR, imu_commands_group_id, imu_accel_command_id,
                                     error_code_to_string(ErrorCode::INVALID_OPERATION)));
        return frames;
    }
    auto &sensors = SensorWrapper::get_instance();
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3)
       << sensors.sensor_read_data(SensorType::IMU, SensorDataTypeIdentifier::ACCEL_X) << ","
       << sensors.sensor_read_data(SensorType::IMU, SensorDataTypeIdentifier::ACCEL_Y) << ","
       << sensors.sensor_read_data(SensorType::IMU, SensorDataTypeIdentifier::ACCEL_Z);
    frames.push_back(frame_build(OperationType::VAL, imu_commands_group_id, imu_accel_command_id, ss.str()));
    return frames;
}

std::vector<Frame> handle_imu_gyro(const std::string &param, OperationType operationType)
{
    std::vector<Frame> frames;
    if (operationType != OperationType::GET || !param.empty())
    {
        frames.push_back(frame_build(OperationType::ERR, imu_commands_group_id, imu_gyro_command_id,
                                     error_code_to_string(ErrorCode::INVALID_OPERATION)));
        return frames;
    }
    auto &sensors = SensorWrapper::get_instance();
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3)
       << sensors.sensor_read_data(SensorType::IMU, SensorDataTypeIdentifier::GYRO_X) << ","
       << sensors.sensor_read_data(SensorType::IMU, SensorDataTypeIdentifier::GYRO_Y) << ","
       << sensors.sensor_read_data(SensorType::IMU, SensorDataTypeIdentifier::GYRO_Z);
    frames.push_back(frame_build(OperationType::VAL, imu_commands_group_id, imu_gyro_command_id, ss.str()));
    return frames;
}
