// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
/**
 * @file telemetry_manager.cpp
 * @brief Implementation of telemetry collection and storage functionality
 * @details Handles collecting, buffering, and persisting telemetry data from various
 *          satellite subsystems including power, sensors, and GPS
 * @defgroup TelemetryManager Telemetry Manager
 * @{
 */

#include "telemetry_manager.h"
#include "utils.h"
#include "storage.h"
#include "PowerManager.h"
#include "ISensor.h"
#include "DS3231.h"
#include <deque>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include "communication.h"
#include "system_state_manager.h"

/**
 * @brief Path to the telemetry CSV file on storage media
 */
#define TELEMETRY_CSV_PATH "/telemetry.csv"

/**
 * @brief Path to the sensor data CSV file on storage media
 */
#define SENSOR_DATA_CSV_PATH "/sensors.csv"

/**
 * @brief Default interval between telemetry samples in milliseconds (2 seconds)
 */
#define DEFAULT_SAMPLE_INTERVAL_MS 1000

/**
 * @brief Default number of records to collect before flushing to storage
 */
#define DEFAULT_FLUSH_THRESHOLD 10

TelemetryManager::TelemetryManager() {}

/**
 * @brief Initializes the telemetry manager.
 * @return True if initialization was successful, false otherwise.
 * @details Initializes the telemetry mutex, checks if the SD card is mounted,
 *          and creates the telemetry and sensor data CSV files if they don't exist.
 *          Also writes the CSV headers to the files.
 * @ingroup TelemetryManager
 */
bool TelemetryManager::init()
{
    mutex_init(&telemetry_mutex);
    if (!SystemStateManager::get_instance().is_sd_card_mounted())
    {
        uart_print("Telemetry system initialized (storage not available)", VerbosityLevel::WARNING);
        return false;
    }

    bool success = true;

    FILE *telemetry_file = fopen(TELEMETRY_CSV_PATH, "w");
    if (!telemetry_file)
    {
        telemetry_file = fopen(TELEMETRY_CSV_PATH, "w");
        if (telemetry_file)
        {
            fprintf(telemetry_file, "timestamp,build,battery_v,system_v,usb_ma,solar_ma,discharge_ma,"
                                    "gps_time,latitude,lat_dir,longitude,lon_dir,speed_mps,course_deg,date,"
                                    "fix_quality,satellites,altitude_m\n");
            fclose(telemetry_file);
            uart_print("Created new telemetry log", VerbosityLevel::INFO);
        }
        else
        {
            uart_print("Failed to create telemetry log", VerbosityLevel::ERROR);
            success = false;
        }
    }
    else
    {
        fclose(telemetry_file);
    }

    FILE *sensor_file = fopen(SENSOR_DATA_CSV_PATH, "w");
    if (!sensor_file)
    {
        sensor_file = fopen(SENSOR_DATA_CSV_PATH, "w");
        if (sensor_file)
        {
            fprintf(sensor_file, "timestamp,temperature,pressure,humidity,light\n");
            fclose(sensor_file);
            uart_print("Created new sensor data log", VerbosityLevel::INFO);
        }
        else
        {
            uart_print("Failed to create sensor data log", VerbosityLevel::ERROR);
            success = false;
        }
    }
    else
    {
        fclose(sensor_file);
    }

    return success;
}

/**
 * @brief Collects power subsystem telemetry data.
 * @param[out] record The telemetry record to update with power data.
 * @ingroup TelemetryManager
 */
void TelemetryManager::collect_power_telemetry(TelemetryRecord &record)
{
    record.battery_voltage = PowerManager::get_instance().get_voltage_battery();
    record.system_voltage = PowerManager::get_instance().get_voltage_5v();
    record.charge_current_usb = PowerManager::get_instance().get_current_charge_usb();
    record.charge_current_solar = PowerManager::get_instance().get_current_charge_solar();
    record.discharge_current = PowerManager::get_instance().get_current_draw();
}

/**
 * @brief Emits power-related events based on current and voltage levels.
 * @param[in] battery_voltage The current battery voltage.
 * @param[in] charge_current_usb The current USB charging current.
 * @param[in] charge_current_solar The current solar charging current.
 * @param[in] discharge_current The current battery discharge current.
 * @ingroup TelemetryManager
 */
void TelemetryManager::emit_power_events(float battery_voltage, float charge_current_usb, float charge_current_solar, float discharge_current)
{
    static bool usb_charging_active = false;
    static bool solar_charging_active = false;
    static bool battery_low = false;
    static bool battery_full = false;
    static bool discharge_active = true;

    if (charge_current_usb > PowerManager::USB_CURRENT_THRESHOLD && !usb_charging_active)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_CONNECTED);
        usb_charging_active = true;
    }
    else if (charge_current_usb < PowerManager::USB_CURRENT_THRESHOLD && usb_charging_active)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::USB_DISCONNECTED);
        usb_charging_active = false;
    }

    if (charge_current_solar > PowerManager::SOLAR_CURRENT_THRESHOLD && !solar_charging_active)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_ACTIVE);
        solar_charging_active = true;
    }
    else if (charge_current_solar < PowerManager::SOLAR_CURRENT_THRESHOLD && solar_charging_active)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::SOLAR_INACTIVE);
        solar_charging_active = false;
    }

    if (battery_voltage < PowerManager::BATTERY_LOW_THRESHOLD && !battery_low)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_LOW);
        battery_low = true;
        battery_full = false;
    }
    else if (battery_voltage > PowerManager::BATTERY_FULL_THRESHOLD && !battery_full)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_FULL);
        battery_full = true;
        battery_low = false;
    }
    else if (battery_voltage > PowerManager::BATTERY_LOW_THRESHOLD && battery_low)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_NORMAL);
        battery_low = false;
    }
    else if (battery_voltage < PowerManager::BATTERY_FULL_THRESHOLD && battery_full)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::BATTERY_NORMAL);
        battery_full = false;
    }

    if (charge_current_solar + charge_current_usb > discharge_current && !discharge_active)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::CHARGING);
        discharge_active = true;
    }
    else if (charge_current_solar + charge_current_usb < discharge_current && discharge_active)
    {
        EventEmitter::emit(EventGroup::POWER, PowerEvent::DISCHARGING);
        discharge_active = false;
    }
}

/**
 * @brief Collects GPS telemetry data.
 * @param[out] record The telemetry record to update with GPS data.
 * @ingroup TelemetryManager
 */
void TelemetryManager::collect_gps_telemetry(TelemetryRecord &record)
{
    auto &nmea_data = NMEAData::get_instance();

    std::vector<std::string> rmc_tokens = nmea_data.get_rmc_tokens();
    if (rmc_tokens.size() >= 12)
    {
        record.time = (rmc_tokens.size() > 1 && !rmc_tokens[1].empty()) ? rmc_tokens[1].substr(0, 6) : "0"; // Only keep HHMMSS
        record.latitude = (rmc_tokens.size() > 3 && !rmc_tokens[3].empty()) ? rmc_tokens[3] : "0";
        record.lat_dir = (rmc_tokens.size() > 4 && !rmc_tokens[4].empty()) ? rmc_tokens[4] : "N";
        record.longitude = (rmc_tokens.size() > 5 && !rmc_tokens[5].empty()) ? rmc_tokens[5] : "0";
        record.lon_dir = (rmc_tokens.size() > 6 && !rmc_tokens[6].empty()) ? rmc_tokens[6] : "E";

        std::string knots = (rmc_tokens.size() > 7 && !rmc_tokens[7].empty()) ? rmc_tokens[7] : "0";
        try
        {
            float speed_knots = std::stof(knots);
            float speed_ms = speed_knots * 0.514444; // Convert knots to m/s
            record.speed = std::to_string(speed_ms);
        }
        catch (const std::exception &)
        {
            record.speed = "0";
        }

        record.course = (rmc_tokens.size() > 8 && !rmc_tokens[8].empty()) ? rmc_tokens[8] : "0";
        record.date = (rmc_tokens.size() > 9 && !rmc_tokens[9].empty()) ? rmc_tokens[9] : "0";
    }
    else
    {
        record.time = "0";
        record.latitude = "0";
        record.lat_dir = "N";
        record.longitude = "0";
        record.lon_dir = "E";
        record.speed = "0";
        record.course = "0";
        record.date = "0";
    }

    std::vector<std::string> gga_tokens = nmea_data.get_gga_tokens();
    if (gga_tokens.size() >= 15)
    {
        record.fix_quality = (gga_tokens.size() > 6 && !gga_tokens[6].empty()) ? gga_tokens[6] : "0";
        record.satellites = (gga_tokens.size() > 7 && !gga_tokens[7].empty()) ? gga_tokens[7] : "0";
        record.altitude = (gga_tokens.size() > 9 && !gga_tokens[9].empty()) ? gga_tokens[9] : "0";
    }
    else
    {
        record.fix_quality = "0";
        record.satellites = "0";
        record.altitude = "0";
    }
}

/**
 * @brief Collects sensor telemetry data.
 * @param[out] sensor_record The sensor data record to update with sensor data.
 * @ingroup TelemetryManager
 */
void TelemetryManager::collect_sensor_telemetry(SensorDataRecord &sensor_record)
{
    SensorWrapper &sensor_wrapper = SensorWrapper::get_instance();
    sensor_record.temperature = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::TEMPERATURE);
    sensor_record.pressure = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::PRESSURE);
    sensor_record.humidity = sensor_wrapper.sensor_read_data(SensorType::ENVIRONMENT, SensorDataTypeIdentifier::HUMIDITY);
    sensor_record.light = sensor_wrapper.sensor_read_data(SensorType::LIGHT, SensorDataTypeIdentifier::LIGHT_LEVEL);
}

/**
 * @brief Collect telemetry data from sensors and power subsystems
 * @return True if data was successfully collected
 * @details Reads data from power manager, sensors, and GPS and stores it
 *          in the telemetry buffer with proper mutex protection
 * @ingroup TelemetryManager
 */
bool TelemetryManager::collect_telemetry()
{
    uint32_t timestamp = DS3231::get_instance().get_local_time();
    TelemetryRecord record;
    record.timestamp = timestamp;
    record.build_version = std::to_string(BUILD_NUMBER);

    collect_power_telemetry(record);
    emit_power_events(record.battery_voltage, record.charge_current_usb, record.charge_current_solar, record.discharge_current);

    collect_gps_telemetry(record);

    SensorDataRecord sensor_record;
    sensor_record.timestamp = timestamp;
    collect_sensor_telemetry(sensor_record);

    mutex_enter_blocking(&telemetry_mutex);

    telemetry_buffer[telemetry_buffer_write_index] = record;
    sensor_data_buffer[telemetry_buffer_write_index] = sensor_record;
    telemetry_buffer_write_index = (telemetry_buffer_write_index + 1) % TELEMETRY_BUFFER_SIZE;
    if (telemetry_buffer_count < TELEMETRY_BUFFER_SIZE)
    {
        telemetry_buffer_count++;
    }

    last_telemetry_record_copy = record;
    last_sensor_record_copy = sensor_record;

    mutex_exit(&telemetry_mutex);

    uart_print("Telemetry collected", VerbosityLevel::DEBUG);

    return true;
}

/**
 * @brief Save buffered telemetry and sensor data to storage
 * @return True if data was successfully saved
 * @details Writes all records from the telemetry and sensor data buffers to their
 *          respective CSV files and clears the buffers after successful writing
 * @ingroup TelemetryManager
 */
bool TelemetryManager::flush_telemetry()
{
    if (!SystemStateManager::get_instance().is_sd_card_mounted())
    {
        return false;
    }

    mutex_enter_blocking(&telemetry_mutex);

    if (telemetry_buffer_count == 0)
    {
        mutex_exit(&telemetry_mutex);
        return true; // Nothing to save
    }

    FILE *telemetry_file = fopen(TELEMETRY_CSV_PATH, "a");
    FILE *sensor_file = fopen(SENSOR_DATA_CSV_PATH, "a");

    if (!telemetry_file || !sensor_file)
    {
        uart_print("Failed to open telemetry or sensor log for writing", VerbosityLevel::ERROR);
        if (telemetry_file)
            fclose(telemetry_file);
        if (sensor_file)
            fclose(sensor_file);
        mutex_exit(&telemetry_mutex);
        return false;
    }

    // Calculate start index (for circular buffer)
    size_t read_index = 0;
    if (telemetry_buffer_count == TELEMETRY_BUFFER_SIZE)
    {
        // Buffer is full, start from oldest entry
        read_index = telemetry_buffer_write_index;
    }

    // Write all records to CSV
    for (size_t i = 0; i < telemetry_buffer_count; i++)
    {
        fprintf(telemetry_file, "%s\n", telemetry_buffer[read_index].to_csv().c_str());
        fprintf(sensor_file, "%s\n", sensor_data_buffer[read_index].to_csv().c_str());
        read_index = (read_index + 1) % TELEMETRY_BUFFER_SIZE;
    }

    // Clear buffer after successful write
    telemetry_buffer_count = 0;
    telemetry_buffer_write_index = 0;

    fclose(telemetry_file);
    fclose(sensor_file);

    mutex_exit(&telemetry_mutex);
    return true;
}

/**
 * @brief Check if it's time to collect telemetry based on interval
 * @param current_time Current system time in milliseconds
 * @param last_collection_time Previous collection time in milliseconds
 * @return True if collection interval has passed
 * @details Updates last_collection_time if the interval has passed
 * @ingroup TelemetryManager
 */
bool TelemetryManager::is_telemetry_collection_time(uint32_t current_time, uint32_t &last_collection_time)
{
    if (current_time - last_collection_time >= sample_interval_ms)
    {
        last_collection_time = current_time;
        return true;
    }
    return false;
}

/**
 * @brief Check if it's time to flush telemetry buffer based on count
 * @param collection_counter Current collection counter
 * @return True if flush threshold has been reached
 * @details Resets collection_counter to zero if the threshold has been reached
 * @ingroup TelemetryManager
 */
bool TelemetryManager::is_telemetry_flush_time(uint32_t &collection_counter)
{
    if (collection_counter >= flush_threshold)
    {
        collection_counter = 0;
        return true;
    }
    return false;
}

/**
 * @brief Gets the last telemetry record as a CSV string.
 * @return A CSV string representing the last telemetry record, or an empty string if no data is available.
 * @ingroup TelemetryManager
 */
std::string TelemetryManager::get_last_telemetry_record_csv()
{
    return last_telemetry_record_copy.to_csv();
}

/**
 * @brief Gets the last sensor data record as a CSV string.
 * @return A CSV string representing the last sensor data record, or an empty string if no data is available.
 * @ingroup TelemetryManager
 */
std::string TelemetryManager::get_last_sensor_record_csv()
{
    return last_sensor_record_copy.to_csv();
}