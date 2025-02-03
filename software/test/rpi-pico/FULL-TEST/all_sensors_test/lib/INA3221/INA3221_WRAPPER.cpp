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
    if (!initialized) return false;

    for (const auto& [key, value] : config) {
        if (key == "operating_mode") {
            if (value == "power_down") {
                sensor.setModePowerDown();
            } else if (value == "continuous") {
                sensor.setModeContinious();
            } else if (value == "triggered") {
                sensor.setModeTriggered();
            } else {
                std::cerr << "Invalid operating_mode: " << value << std::endl;
                return false;
            }
        }
        else if (key == "averaging_mode") {
            if (value == "1") sensor.setAveragingMode(INA3221_REG_CONF_AVG_1);
            else if (value == "4") sensor.setAveragingMode(INA3221_REG_CONF_AVG_4);
            else if (value == "16") sensor.setAveragingMode(INA3221_REG_CONF_AVG_16);
            else if (value == "64") sensor.setAveragingMode(INA3221_REG_CONF_AVG_64);
            else if (value == "128") sensor.setAveragingMode(INA3221_REG_CONF_AVG_128);
            else if (value == "256") sensor.setAveragingMode(INA3221_REG_CONF_AVG_256);
            else if (value == "512") sensor.setAveragingMode(INA3221_REG_CONF_AVG_512);
            else if (value == "1024") sensor.setAveragingMode(INA3221_REG_CONF_AVG_1024);
        }
        else if (key == "conversion_time_bus") {
            if (value == "140us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_140US);
            else if (value == "204us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_204US);
            else if (value == "332us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_332US);
            else if (value == "588us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_588US);
            else if (value == "1100us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_1100US);
            else if (value == "2116us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_2116US);
            else if (value == "4156us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_4156US);
            else if (value == "8244us") sensor.setBusConversionTime(INA3221_REG_CONF_CT_8244US);
        }
        else if (key == "conversion_time_shunt") {
            if (value == "140us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_140US);
            else if (value == "204us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_204US);
            else if (value == "332us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_332US);
            else if (value == "588us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_588US);
            else if (value == "1100us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_1100US);
            else if (value == "2116us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_2116US);
            else if (value == "4156us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_4156US);
            else if (value == "8244us") sensor.setShuntConversionTime(INA3221_REG_CONF_CT_8244US);
        }
        else {
            std::cerr << "Unknown configuration key: " << key << std::endl;
            return false;
        }
    }
    return true;
}