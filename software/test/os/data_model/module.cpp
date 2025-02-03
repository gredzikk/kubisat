#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "enum.cpp"
// Enum for sensor types identified by hex values


// Uniform structure to hold sensor data
struct SensorData {
    SensorType sensorType;
    std::vector<std::pair<std::string, double>> parameters;
    std::string unit;
    double scaler;
    size_t size;
};

// Base class for sensors
class Sensor {
public:
    virtual ~Sensor() = default;
    virtual SensorData readParameters() = 0;
};

// Derived class for INA3221
class INA3221 : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::INA3221;
        data.parameters.push_back({"Voltage", 3.3});
        data.parameters.push_back({"Current", 1.2});
        data.unit = "V";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for MPU6050
class MPU6050 : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::MPU6050;
        data.parameters.push_back({"AccelerationX", 0.02});
        data.parameters.push_back({"AccelerationY", 0.01});
        data.parameters.push_back({"AccelerationZ", 0.98});
        data.unit = "g";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for BMP280
class BMP280 : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::BMP280;
        data.parameters.push_back({"Temperature", 25.0});
        data.parameters.push_back({"Pressure", 1013.25});
        data.unit = "hPa";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for NEO6M
class NEO6M : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::NEO6M;
        data.parameters.push_back({"Latitude", 52.5200});
        data.parameters.push_back({"Longitude", 13.4050});
        data.unit = "degrees";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for SX1278
class SX1278 : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::SX1278;
        data.parameters.push_back({"Frequency", 868.0});
        data.parameters.push_back({"SignalStrength", -70});
        data.unit = "dBm";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for MICROSD
class MICROSD : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::MICROSD;
        data.parameters.push_back({"StorageCapacity", 32.0});
        data.parameters.push_back({"FreeSpace", 10.0});
        data.unit = "GB";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for EEPROM
class EEPROM : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::EEPROM;
        data.parameters.push_back({"Size", 256.0});
        data.parameters.push_back({"UsedSpace", 128.0});
        data.unit = "KB";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Derived class for QMC5883L
class QMC5883L : public Sensor {
public:
    SensorData readParameters() override {
        SensorData data;
        data.sensorType = SensorType::QMC5883L;
        data.parameters.push_back({"MagneticFieldX", 0.1});
        data.parameters.push_back({"MagneticFieldY", 0.2});
        data.parameters.push_back({"MagneticFieldZ", 0.3});
        data.unit = "uT";
        data.scaler = 1.0;
        data.size = sizeof(data);
        return data;
    }
};

// Function to retrieve sensor data based on sensor identifier and value type
std::string getSensorData(const std::vector<std::unique_ptr<Sensor>>& sensors, SensorType sensorType, ValueType valueType) {
    for (const auto& sensor : sensors) {
        SensorData data = sensor->readParameters();
        if (data.sensorType == sensorType) {
            switch (valueType) {
                case ValueType::RawValue:
                    return "Raw Value: " + std::to_string(data.parameters[0].second); // Example for first parameter
                case ValueType::Type:
                    return "Type: " + std::to_string(static_cast<uint8_t>(data.sensorType));
                case ValueType::Size:
                    return "Size: " + std::to_string(data.size);
                case ValueType::Unit:
                    return "Unit: " + data.unit;
                case ValueType::Scaler:
                    return "Scaler: " + std::to_string(data.scaler);
                default:
                    return "Invalid ValueType";
            }
        }
    }
    return "Sensor not found";
}

// Example usage
int main() {
    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.push_back(std::make_unique<INA3221>());
    sensors.push_back(std::make_unique<MPU6050>());
    sensors.push_back(std::make_unique<BMP280>());
    sensors.push_back(std::make_unique<NEO6M>());
    sensors.push_back(std::make_unique<SX1278>());
    sensors.push_back(std::make_unique<MICROSD>());
    sensors.push_back(std::make_unique<EEPROM>());
    sensors.push_back(std::make_unique<QMC5883L>());

    // Retrieve and print sensor data
    std::cout << getSensorData(sensors, SensorType::INA3221, ValueType::RawValue) << std::endl;
    std::cout << getSensorData(sensors, SensorType::MPU6050, ValueType::Unit) << std::endl;

    return 0;
}