#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>

enum class SensorType : uint8_t {
    INA3221 = 0x01,
    MPU6050 = 0x02,
    BMP280 = 0x03,
    NEO6M = 0x04,
    SX1278 = 0x05,
    MICROSD = 0x06,
    EEPROM = 0x07,
    QMC5883L = 0x08
};

enum class ValueType : uint8_t {
    RawValue = 0x01,
    Type = 0x02,
    Size = 0x03,
    Unit = 0x04,
    Scaler = 0x05
};

enum class DataType : uint8_t {
    Structure = 0x01,
    Array = 0x02,
    Enum = 0x03,
    Bool = 0x04,
    HexString = 0x05,
    Integer = 0x06,
    Double = 0x07,
    Position = 0x08,
    Time = 0x09,
    Date = 0x0A,
    Undefined = 0xFF
};

enum class Unit : uint8_t {
    Volt = 0x01,
    Ampere = 0x02,
    Celsius = 0x03,
    Pascal = 0x04,
    Degree = 0x05,
    Gal = 0x06, //  980 Gal = 9,8m/s^2
    Tesla = 0x07,
    Meter = 0x08,
    KPH = 0x09,    
    Undefined = 0xFF
};

struct SensorData {
    DataType type;
    uint8_t size;
    uint16_t intValue;
    bool boolValue;
    uint8_t enumValue;
};

std::string encodeData(const SensorData& data) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    oss << static_cast<uint8_t>(data.type);
    oss << std::setw(2) << static_cast<int>(data.size);
    oss << std::setw(4) << data.intValue;
    oss << std::setw(2) << static_cast<int>(data.boolValue);
    oss << std::setw(2) << static_cast<int>(data.enumValue);
    return oss.str();
}

SensorData getSensorData(SensorType sensor) {
    // Example data, this should be replaced with actual sensor data retrieval logic
    if (sensor == SensorType::QMC5883L) {
        return { DataType::Structure, 3, 0x0009, true, 0x03 };
    }
    // Add more cases for other sensors
    return { DataType::Undefined, 0, 0, false, 0 };
}

std::string requestSensorData(SensorType sensor) {
    SensorData data = getSensorData(sensor);
    return encodeData(data);
}

int main() {
    SensorType sensor = SensorType::QMC5883L;
    std::string encodedData = requestSensorData(sensor);
    std::cout << "Encoded Data: " << encodedData << std::endl;
    return 0;
}