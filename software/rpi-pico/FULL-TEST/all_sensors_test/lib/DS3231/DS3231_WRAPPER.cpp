#include "DS3231_WRAPPER.h"
#include <iostream>

DS3231Wrapper::DS3231Wrapper(i2c_inst_t* i2c) : sensor(i2c) {}

bool DS3231Wrapper::init() {
    try {
        int32_t result = sensor.getDatetime(&current_time);
        initialized = (result == 0);
        return initialized;
    } catch (...) {
        initialized = false;
        return false;
    }
}

float DS3231Wrapper::readData(DataType type) {
    if (!initialized) return 0.0f;

    if (type == DataType::TIMESTAMP) {
        if (sensor.getDatetime(&current_time) == 0) {
            // Simplified Unix timestamp calculation
            return static_cast<float>(
                current_time.year * 31536000 +
                current_time.month * 2592000 +
                current_time.day * 86400 +
                current_time.hour * 3600 +
                current_time.min * 60 +
                current_time.sec
            );
        }
    }
    return 0.0f;
}

bool DS3231Wrapper::isInitialized() const {
    return initialized;
}

SensorType DS3231Wrapper::getType() const {
    return SensorType::TIME;
}

bool DS3231Wrapper::setTime(datetime_t* dt) {
    if (!initialized) return false;
    return sensor.setDatetime(dt) == 0;
}

bool DS3231Wrapper::configure(const std::map<std::string, std::string>& config) {
    if (!initialized) return false;
    
    // Get current time first
    if (!sensor.getDatetime(&current_time)) {
        std::cerr << "Failed to get current time" << std::endl;
        return false;
    }

    datetime_t new_time = current_time; // Start with current values
    bool time_modified = false;

    for (const auto& [key, value] : config) {
        try {
            if (key == "year") {
                new_time.year = std::stoi(value);
                time_modified = true;
            }
            else if (key == "month") {
                new_time.month = std::stoi(value);
                time_modified = true;
            }
            else if (key == "day") {
                new_time.day = std::stoi(value);
                time_modified = true;
            }
            else if (key == "hour") {
                new_time.hour = std::stoi(value);
                time_modified = true;
            }
            else if (key == "minute") {
                new_time.min = std::stoi(value);
                time_modified = true;
            }
            else if (key == "second") {
                new_time.sec = std::stoi(value);
                time_modified = true;
            }
            else if (key == "dotw") {
                new_time.dotw = std::stoi(value);
                time_modified = true;
            }
            else {
                std::cerr << "Unknown configuration key: " << key << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing time value for " << key << ": " << e.what() << std::endl;
            return false;
        }
    }

    if (time_modified) {
        // Calculate day of week if not provided
        if (config.find("dotw") == config.end()) {
            new_time.dotw = getDayOfWeek(new_time.year, new_time.month, new_time.day);
        }
        return setTime(&new_time);
    }

    return true;
}