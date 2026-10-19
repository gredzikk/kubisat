// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

/**
 * @file telemetry_manager.h
 * @brief System telemetry collection and logging
 * @details This module handles periodic collection and storage of telemetry
 *          data from various satellite subsystems including power management,
 *          sensors (temperature, pressure, humidity, light), and GPS data.
 *
 *          Telemetry is collected at configurable intervals and stored in a
 *          circular buffer before being flushed to persistent storage after
 *          a configurable number of records are collected.
 *
 * @defgroup TelemetryManager Telemetry Manager
 * @{
 */

#ifndef TELEMETRY_MANAGER_H
#define TELEMETRY_MANAGER_H

#include <cstdint>
#include <string>
#include "pico/stdlib.h"
#include "lib/location/NMEA/NMEA_data.h"
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
#include <array>
#include "communication.h"
#include <functional>

/**
 * @struct TelemetryRecord
 * @brief Structure representing a single telemetry data point
 * @details Contains all measurements from power subsystem, sensors, and GPS data
 *          collected at a specific point in time
 */
struct TelemetryRecord
{
    uint32_t timestamp; /**< Unix timestamp of the record */

    std::string build_version; /**< Build version of the firmware */

    // Power data
    float battery_voltage;      /**< Battery voltage in volts */
    float system_voltage;       /**< System 5V rail voltage in volts */
    float charge_current_usb;   /**< USB charging current in mA */
    float charge_current_solar; /**< Solar charging current in mA */
    float discharge_current;    /**< Battery discharge current in mA */

    // GPS data - key RMC fields
    std::string time;      /**< UTC time from GPS */
    std::string latitude;  /**< Latitude from GPS */
    std::string lat_dir;   /**< N/S latitude direction */
    std::string longitude; /**< Longitude from GPS */
    std::string lon_dir;   /**< E/W longitude direction */
    std::string speed;     /**< Speed in knots */
    std::string course;    /**< Course in degrees */
    std::string date;      /**< Date from GPS */

    // GPS data - key GGA fields
    std::string fix_quality; /**< GPS fix quality */
    std::string satellites;  /**< Number of satellites in view */
    std::string altitude;    /**< Altitude in meters */

    /**
     * @brief Converts the telemetry record to a CSV string.
     * @return A CSV string representing the telemetry record.
     * @ingroup TelemetryManager
     */
    std::string to_csv() const
    {
        std::stringstream ss;
        ss << timestamp << ","
           << build_version << ","
           << std::fixed << std::setprecision(3)
           << battery_voltage << ","
           << system_voltage << ","
           << charge_current_usb << ","
           << charge_current_solar << ","
           << discharge_current << ","

           // GPS RMC data
           << time << ","
           << latitude << "," << lat_dir << ","
           << longitude << "," << lon_dir << ","
           << speed << ","
           << course << ","
           << date << ","
           // GPS GGA data
           << fix_quality << ","
           << satellites << ","
           << altitude;
        return ss.str();
    }
};

/**
 * @struct SensorDataRecord
 * @brief Structure representing a single sensor data point
 * @details Contains measurements from the environment and light sensors
 *          collected at a specific point in time
 * @ingroup TelemetryManager
 */
struct SensorDataRecord
{
    uint32_t timestamp; /**< Unix timestamp of the record */
    float temperature;  /**< Temperature in degrees Celsius */
    float pressure;     /**< Pressure in hPa */
    float humidity;     /**< Relative humidity in % */
    float light;        /**< Light intensity in lux */
    float accel_x;      /**< Acceleration X (g) */
    float accel_y;      /**< Acceleration Y (g) */
    float accel_z;      /**< Acceleration Z (g) */
    float gyro_x;       /**< Gyro X (dps) */
    float gyro_y;       /**< Gyro Y (dps) */
    float gyro_z;       /**< Gyro Z (dps) */

    std::string to_csv() const
    {
        std::stringstream ss;
        ss << timestamp << ","
           << std::fixed << std::setprecision(3)
           << temperature << ","
           << pressure << ","
           << humidity << ","
           << light << ","
           << accel_x << "," << accel_y << "," << accel_z << ","
           << gyro_x << "," << gyro_y << "," << gyro_z;
        return ss.str();
    }
};

/**
 * @class TelemetryManager
 * @brief Manages the collection, storage, and retrieval of telemetry data.
 * @details This class implements a singleton pattern to provide a single
 *          point of access for managing telemetry data. It handles the
 *          collection of data from various subsystems, stores the data in
 *          circular buffers, and provides methods for flushing the data to
 *          persistent storage and retrieving the last recorded data.
 * @ingroup TelemetryManager
 */
class TelemetryManager
{
public:
    /**
     * @brief Gets the singleton instance of the TelemetryManager class.
     * @return A reference to the singleton instance.
     */
    static TelemetryManager &get_instance()
    {
        static TelemetryManager instance;
        return instance;
    }

    /**
     * @brief Initialize the telemetry system
     * @return True if initialization was successful
     * @details Sets up the mutex for thread-safe buffer access and creates a telemetry
     *          CSV file with appropriate headers if it doesn't already exist
     */
    bool init();

    /**
     * @brief Collect telemetry data from sensors and power subsystems
     * @return True if data was successfully collected
     * @details Reads data from power manager, sensors, and GPS and stores it
     *          in the telemetry buffer with proper mutex protection
     */
    bool collect_telemetry();

    /**
     * @brief Collects power subsystem telemetry data.
     * @param[out] record The telemetry record to update with power data.
     * @ingroup TelemetryManager
     */
    void collect_power_telemetry(TelemetryRecord &record);

    /**
     * @brief Emits power-related events based on current and voltage levels.
     * @param[in] battery_voltage The current battery voltage.
     * @param[in] charge_current_usb The current USB charging current.
     * @param[in] charge_current_solar The current solar charging current.
     * @param[in] discharge_current The current battery discharge current.
     * @ingroup TelemetryManager
     */
    void emit_power_events(float battery_voltage, float charge_current_usb, float charge_current_solar, float discharge_current);

    /**
     * @brief Collects GPS telemetry data.
     * @param[out] record The telemetry record to update with GPS data.
     * @ingroup TelemetryManager
     */
    void collect_gps_telemetry(TelemetryRecord &record);

    /**
     * @brief Collects sensor telemetry data.
     * @param[out] sensor_record The sensor data record to update with sensor data.
     * @ingroup TelemetryManager
     */
    void collect_sensor_telemetry(SensorDataRecord &sensor_record);

    /**
     * @brief Save buffered telemetry data to storage
     * @return True if data was successfully saved
     * @details Writes all records from the telemetry buffer to the CSV file
     *          and clears the buffer after successful writing
     */
    bool flush_telemetry();

    /**
     * @brief Check if it's time to collect telemetry based on interval
     * @param current_time Current system time in milliseconds
     * @param last_collection_time Previous collection time in milliseconds
     * @return True if collection interval has passed
     * @details Updates last_collection_time if the interval has passed
     */
    bool is_telemetry_collection_time(uint32_t current_time, uint32_t &last_collection_time);

    /**
     * @brief Check if it's time to flush telemetry buffer based on count
     * @param collection_counter Current collection counter
     * @return True if flush threshold has been reached
     * @details Resets collection_counter to zero if the threshold has been reached
     */
    bool is_telemetry_flush_time(uint32_t &collection_counter);

    /**
     * @brief Gets the last telemetry record as a CSV string.
     * @return A CSV string representing the last telemetry record, or an empty string if no data is available.
     */
    std::string get_last_telemetry_record_csv();

    /**
     * @brief Gets the last sensor data record as a CSV string.
     * @return A CSV string representing the last sensor data record, or an empty string if no data is available.
     */
    std::string get_last_sensor_record_csv();

    /** @brief Returns up to N newest telemetry CSV lines separated by '|'. */
    std::string get_recent_telemetry_csv(size_t count);

    static constexpr int TELEMETRY_BUFFER_SIZE = 20;

    size_t get_telemetry_buffer_count() const { return telemetry_buffer_count; }
    size_t get_telemetry_buffer_write_index() const { return telemetry_buffer_write_index; }

private:
    TelemetryManager(); // Private constructor
    ~TelemetryManager() = default;

    /**
     * @brief Current sampling interval in milliseconds
     */
    static constexpr uint32_t DEFAULT_SAMPLE_INTERVAL_MS = 1000;

    /**
     * @brief Default number of records to collect before flushing to storage
     */
    static constexpr uint32_t DEFAULT_FLUSH_THRESHOLD = 10;

    uint32_t sample_interval_ms = DEFAULT_SAMPLE_INTERVAL_MS;

    /**
     * @brief Current flush threshold (number of records that triggers a flush)
     */
    uint32_t flush_threshold = DEFAULT_FLUSH_THRESHOLD;
    /**
     * @brief Circular buffer for telemetry records
     */
    std::array<TelemetryRecord, TELEMETRY_BUFFER_SIZE> telemetry_buffer;
    size_t telemetry_buffer_count = 0;
    size_t telemetry_buffer_write_index = 0;

    /**
     * @brief Circular buffer for sensor data records
     */
    std::array<SensorDataRecord, TELEMETRY_BUFFER_SIZE> sensor_data_buffer;

    /**
     * @brief Last record copies for retrieval
     */
    TelemetryRecord last_telemetry_record_copy;
    SensorDataRecord last_sensor_record_copy;

    /**
     * @brief Mutex for thread-safe access to the telemetry buffer
     */
    mutex_t telemetry_mutex;
};
#endif // TELEMETRY_MANAGER_H

/** @} */ // End of TelemetryManager group