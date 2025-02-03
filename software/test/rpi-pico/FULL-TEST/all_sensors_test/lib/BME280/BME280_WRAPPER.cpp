#include "BME280_WRAPPER.h"

BME280Wrapper::BME280Wrapper(i2c_inst_t* i2c) : sensor(i2c) {}

bool BME280Wrapper::init() {
    initialized = sensor.init();
    return initialized;
}

float BME280Wrapper::readData(DataType type) {
    int32_t temp_raw, pressure_raw, humidity_raw;
    sensor.read_raw_all(&temp_raw, &pressure_raw, &humidity_raw);

    switch(type) {
        case DataType::TEMPERATURE:
            return sensor.convert_temperature(temp_raw);
        case DataType::PRESSURE:
            return sensor.convert_pressure(pressure_raw);
        case DataType::HUMIDITY:
            return sensor.convert_humidity(humidity_raw);
        default:
            return 0.0f;
    }
}

bool BME280Wrapper::isInitialized() const {
    return initialized;
}

SensorType BME280Wrapper::getType() const {
    return SensorType::ENVIRONMENT;
}

bool BME280Wrapper::configure(const std::map<std::string, std::string>& config) {
    return true;
}