// ISensor.h
#ifndef ISENSOR_H
#define ISENSOR_H

#include <cstdint>
#include "hardware/i2c.h"
#include "hardware/spi.h"

enum class SensorType {
    LIGHT,      // BH1750
    CURRENT,    // INA3221
    TIME,       // DS3231
    ENVIRONMENT,// BME280
    RADIO       // LoRa
};

enum class DataType {
    LIGHT_LEVEL,
    CURRENT_CH1,
    CURRENT_CH2,
    CURRENT_CH3,
    VOLTAGE_CH1,
    VOLTAGE_CH2,
    VOLTAGE_CH3,
    TIMESTAMP,
    TEMPERATURE,
    PRESSURE,
    HUMIDITY,
    RADIO_RSSI,
    RADIO_SNR
};

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool init() = 0;
    virtual float readData(DataType type) = 0;
    virtual bool isInitialized() const = 0;
    virtual SensorType getType() const = 0;
};

// SensorWrapper implementation
class SensorWrapper {
public:
    static SensorWrapper& getInstance() {
        static SensorWrapper instance;
        return instance;
    }

    bool initSensor(SensorType type, i2c_inst_t* i2c = nullptr, spi_inst_t* spi = nullptr) {
        switch(type) {
            case SensorType::LIGHT:
                sensors[type] = new BH1750Wrapper();
                break;
            case SensorType::CURRENT:
                sensors[type] = new INA3221Wrapper(i2c);
                break;
            case SensorType::TIME:
                sensors[type] = new DS3231Wrapper(i2c);
                break;
            case SensorType::ENVIRONMENT:
                sensors[type] = new BME280Wrapper(i2c);
                break;
            case SensorType::RADIO:
                sensors[type] = new LoRaWrapper(spi);
                break;
        }
        return sensors[type]->init();
    }

    float readSensorData(SensorType sensorType, DataType dataType) {
        auto it = sensors.find(sensorType);
        if (it != sensors.end() && it->second->isInitialized()) {
            return it->second->readData(dataType);
        }
        return 0.0f;
    }

private:
    std::map<SensorType, ISensor*> sensors;
    SensorWrapper() = default;
};

// Example wrapper for BH1750
class BH1750Wrapper : public ISensor {
private:
    BH1750 sensor;
    bool initialized = false;

public:
    BH1750Wrapper() : sensor(BH1750_DEFAULT_ADDRESS) {}

    bool init() override {
        initialized = sensor.begin();
        return initialized;
    }

    float readData(DataType type) override {
        if (type == DataType::LIGHT_LEVEL) {
            return sensor.readLightLevel();
        }
        return 0.0f;
    }

    bool isInitialized() const override { return initialized; }
    SensorType getType() const override { return SensorType::LIGHT; }
};

// Example wrapper for BME280
class BME280Wrapper : public ISensor {
private:
    BMP280 sensor;
    bool initialized = false;

public:
    BME280Wrapper(i2c_inst_t* i2c) : sensor(i2c) {}

    bool init() override {
        initialized = sensor.init();
        return initialized;
    }

    float readData(DataType type) override {
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

    bool isInitialized() const override { return initialized; }
    SensorType getType() const override { return SensorType::ENVIRONMENT; }
};
