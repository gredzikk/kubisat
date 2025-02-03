// ISensor.cpp
#include "ISensor.h"
#include "lib/BH1750/BH1750_WRAPPER.h"
#include "lib/INA3221/INA3221_WRAPPER.h"
#include "lib/DS3231/DS3231_WRAPPER.h"
#include "lib/BME280/BME280_WRAPPER.h"

SensorWrapper& SensorWrapper::getInstance() {
    static SensorWrapper instance;
    return instance;
}

SensorWrapper::SensorWrapper() = default;

bool SensorWrapper::initSensor(SensorType type, i2c_inst_t* i2c) {
    switch(type) {
        case SensorType::LIGHT:
            sensors[type] = new BH1750Wrapper();
            break;
        case SensorType::POWER:
            sensors[type] = new INA3221Wrapper(i2c);
            break;
        case SensorType::TIME:
            sensors[type] = new DS3231Wrapper(i2c);
            break;
        case SensorType::ENVIRONMENT:
            sensors[type] = new BME280Wrapper(i2c);
            break;
    }
    return sensors[type]->init();
}

bool SensorWrapper::configureSensor(SensorType type, const std::map<std::string, std::string>& config) {
    auto it = sensors.find(type);
    if (it != sensors.end() && it->second->isInitialized()) {
        return it->second->configure(config);
    }
    std::cerr << "Sensor not initialized or not found: " << static_cast<int>(type) << std::endl;
    return false;
}

float SensorWrapper::readSensorData(SensorType sensorType, DataType dataType) {
    auto it = sensors.find(sensorType);
    if (it != sensors.end() && it->second->isInitialized()) {
        return it->second->readData(dataType);
    }
    return 0.0f;
}