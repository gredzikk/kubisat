// BH1750Wrapper.cpp
#include "BH1750_WRAPPER.h"
#include <string>
#include <iostream>

BH1750Wrapper::BH1750Wrapper() {
    sensor.configure(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE);
}

bool BH1750Wrapper::init() {
    initialized = sensor.begin();
    return initialized;
}

float BH1750Wrapper::readData(DataType type) {
    if (type == DataType::LIGHT_LEVEL) {
        return sensor.readLightLevel();
    }
    return 0.0f;
}

bool BH1750Wrapper::isInitialized() const { 
    return initialized; 
}

SensorType BH1750Wrapper::getType() const { 
    return SensorType::LIGHT; 
}

bool BH1750Wrapper::configure(const std::map<std::string, std::string>& config) {
    for (const auto& [key, value] : config) {
        if (key == "measurement_mode") {
            if (value == "continuously_high_resolution") {
                sensor.configure(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE);
            }
            else if (value == "continuously_high_resolution_2") {
                sensor.configure(BH1750::Mode::CONTINUOUS_HIGH_RES_MODE_2);
            }
            else if (value == "continuously_low_resolution") {
                sensor.configure(BH1750::Mode::CONTINUOUS_LOW_RES_MODE);
            }
            else if (value == "one_time_high_resolution") {
                sensor.configure(BH1750::Mode::ONE_TIME_HIGH_RES_MODE);
            }
            else if (value == "one_time_high_resolution_2") {
                sensor.configure(BH1750::Mode::ONE_TIME_HIGH_RES_MODE_2);
            }
            else if (value == "one_time_low_resolution") {
                sensor.configure(BH1750::Mode::ONE_TIME_LOW_RES_MODE);
            }
            else {
                std::cerr << "[BH1750Wrapper] Unknown measurement_mode value: " << value << std::endl;
                return false;
            }
        }
        // Handle additional configuration keys here
        else {
            std::cerr << "[BH1750Wrapper] Unknown configuration key: " << key << std::endl;
            return false;
        }
    }
    return true;
}