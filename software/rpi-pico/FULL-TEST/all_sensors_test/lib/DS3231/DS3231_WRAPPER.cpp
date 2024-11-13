#include "DS3231_WRAPPER.h"

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
    return true;
}