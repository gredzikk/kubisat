#include "INA3221_WRAPPER.h"

INA3221Wrapper::INA3221Wrapper(i2c_inst_t* i2c) : sensor(INA3221_ADDR40_GND) {}

bool INA3221Wrapper::init() {
    try {
        sensor.begin();
        initialized = true;
        return true;
    } catch (...) {
        initialized = false;
        return false;
    }
}

float INA3221Wrapper::readData(DataType type) {
    if (!initialized) return 0.0f;

    switch (type) {
        case DataType::CURRENT_CH1:
            return sensor.getCurrent_mA(INA3221_CH1);
        case DataType::CURRENT_CH2:
            return sensor.getCurrent_mA(INA3221_CH2);
        case DataType::CURRENT_CH3:
            return sensor.getCurrent_mA(INA3221_CH3);
        case DataType::VOLTAGE_CH1:
            return sensor.getVoltage(INA3221_CH1);
        case DataType::VOLTAGE_CH2:
            return sensor.getVoltage(INA3221_CH2);
        case DataType::VOLTAGE_CH3:
            return sensor.getVoltage(INA3221_CH3);
        default:
            return 0.0f;
    }
}

bool INA3221Wrapper::isInitialized() const {
    return initialized;
}

SensorType INA3221Wrapper::getType() const {
    return SensorType::POWER;
}

bool INA3221Wrapper::configure(const std::map<std::string, std::string>& config) {
    return true;
}