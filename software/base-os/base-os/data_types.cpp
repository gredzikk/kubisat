#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <map>
#include <cmath>

enum class Unit {
    UNDEFINED = 0,
    TESLA = 1,
    DEGREE_PER_SECOND = 2,
    AMPERE = 3,
    VOLT = 4,
    DEGREE_CELSIUS = 5,
    PASCAL = 6,
    HHMMSS_SSS = 7,
    DEGREE_NORTH = 8,
    DEGREE_SOUTH = 9,
    DEGREE_EAST = 10,
    DEGREE_WEST = 11,
    ENUM_INTEGER = 12,
    METER = 13,
    DEGREE = 14,
    KILOMETER_PER_HOUR = 15
};

enum class Scaler {
    MINUS_FIVE = 0,
    MINUS_FOUR = 1,
    MINUS_THREE = 2,
    MINUS_TWO = 3,
    MINUS_ONE = 4,
    ZERO = 5,
    ONE = 6,
    TWO = 7,
    THREE = 8,
    FOUR = 9,
    FIVE = 10
};

enum class DataType {
    UNDEFINED = 0,
    INTEGER = 1,
    HEX_STRING = 2,
    BIT_STRING = 3,
    ENUM = 4
};

enum class SensorType {
    RESERVED = 0,
    HMC5883L = 1,
    INA3221 = 2,
    BMP280 = 3,
    MPU6050 = 4,
    NEO_6M = 5
};

enum class DataRegister {
    DATA_REGISTER_0 = 0,
    DATA_REGISTER_1 = 1,
    DATA_REGISTER_2 = 2,
    DATA_REGISTER_3 = 3,
    DATA_REGISTER_4 = 4,
    DATA_REGISTER_5 = 5,
    DATA_REGISTER_6 = 6,
    DATA_REGISTER_7 = 7,
    DATA_REGISTER_8 = 8,
    DATA_REGISTER_9 = 9,
    DATA_REGISTER_10 = 10,
    DATA_REGISTER_11 = 11,
    DATA_REGISTER_12 = 12
};

class Sensor {
public:
    Sensor(SensorType sensor_type, std::map<DataRegister, std::string> registers)
        : sensor_type(sensor_type), registers(registers) {}

    SensorType sensor_type;
    std::map<DataRegister, std::string> registers;
};

struct SensorData {
    SensorType sensor_type;
    DataRegister register_;
    DataType data_type;
    std::vector<int> value;
};

struct IntegerSensorData : public SensorData {
    Unit unit_type;
    Scaler scaler;
};

struct StringSensorData : public SensorData {
    int length;
};

std::vector<int> value_to_list(int value, DataType data_type) {
    std::vector<int> result;
    if (data_type == DataType::INTEGER) {
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << value;
        std::string hex_value = ss.str();
        for (size_t i = 0; i < hex_value.size(); i += 2) {
            result.push_back(std::stoi(hex_value.substr(i, 2), nullptr, 16));
        }
    }
    else if (data_type == DataType::BIT_STRING) {
        std::string bit_string = std::bitset<32>(value).to_string();
        for (size_t i = 0; i < bit_string.size(); i += 4) {
            result.push_back(std::stoi(bit_string.substr(i, 4), nullptr, 2));
        }
    }
    else if (data_type == DataType::HEX_STRING) {
        std::stringstream ss;
        ss << std::hex << value;
        std::string hex_value = ss.str();
        for (size_t i = 0; i < hex_value.size(); i += 2) {
            result.push_back(std::stoi(hex_value.substr(i, 2), nullptr, 16));
        }
    }
    else {
        throw std::invalid_argument("Unsupported data type");
    }
    return result;
}

std::string create_logical_name(Sensor sensor, DataRegister register_) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sensor.sensor_type)
        << std::setw(2) << std::setfill('0') << static_cast<int>(register_);
    return ss.str();
}

Sensor* get_sensor_class(SensorType sensor_type) {
    // Placeholder for actual sensor class retrieval
    return nullptr;
}

std::string serialize_sensor_data(SensorData* sensor_data) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sensor_data->sensor_type)
        << std::setw(2) << std::setfill('0') << static_cast<int>(sensor_data->register_)
        << std::setw(2) << std::setfill('0') << static_cast<int>(sensor_data->data_type);

    if (auto* string_data = dynamic_cast<StringSensorData*>(sensor_data)) {
        ss << std::setw(2) << std::setfill('0') << string_data->length;
        for (int v : string_data->value) {
            ss << std::setw(2) << std::setfill('0') << v;
        }
    }
    else if (auto* int_data = dynamic_cast<IntegerSensorData*>(sensor_data)) {
        for (int v : int_data->value) {
            ss << std::setw(2) << std::setfill('0') << v;
        }
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(int_data->unit_type)
            << std::setw(2) << std::setfill('0') << static_cast<int>(int_data->scaler);
    }
    else {
        throw std::invalid_argument("Unsupported SensorData type");
    }

    return ss.str();
}

std::string read_sensor_data(SensorType sensor_type, DataRegister register_) {
    Sensor* sensor = get_sensor_class(sensor_type);
    if (!sensor) {
        throw std::invalid_argument("Unsupported sensor type");
    }

    auto it = sensor->registers.find(register_);
    if (it == sensor->registers.end()) {
        throw std::invalid_argument("Unsupported register for sensor type");
    }

    // Placeholder for actual sensor data reading
    SensorData* sensor_data = nullptr;

    return serialize_sensor_data(sensor_data);
}

SensorData* deserialize_sensor_data(const std::string& serialized_data) {
    SensorType sensor_type = static_cast<SensorType>(std::stoi(serialized_data.substr(0, 2), nullptr, 16));
    DataRegister register_ = static_cast<DataRegister>(std::stoi(serialized_data.substr(2, 2), nullptr, 16));
    DataType data_type = static_cast<DataType>(std::stoi(serialized_data.substr(4, 2), nullptr, 16));

    if (data_type == DataType::HEX_STRING) {
        int length = std::stoi(serialized_data.substr(6, 2), nullptr, 16);
        std::vector<int> value;
        for (size_t i = 8; i < 8 + length * 2; i += 2) {
            value.push_back(std::stoi(serialized_data.substr(i, 2), nullptr, 16));
        }
        return new StringSensorData{ sensor_type, register_, data_type, value, length };
    }
    else if (data_type == DataType::BIT_STRING) {
        int length = std::stoi(serialized_data.substr(6, 2), nullptr, 16);
        std::vector<int> value;
        for (size_t i = 8; i < 8 + length * 2; i += 2) {
            value.push_back(std::stoi(serialized_data.substr(i, 2), nullptr, 2));
        }
        return new StringSensorData{ sensor_type, register_, data_type, value, length };
    }
    else {
        int value = std::stoi(serialized_data.substr(6, 8), nullptr, 16);
        Unit unit_type = static_cast<Unit>(std::stoi(serialized_data.substr(14, 2), nullptr, 16));
        Scaler scaler = static_cast<Scaler>(std::stoi(serialized_data.substr(16, 2), nullptr, 16));
        return new IntegerSensorData{ sensor_type, register_, data_type, {value}, unit_type, scaler };
    }
}

std::string pretty_print_sensor_data(SensorData* sensor_data) {
    std::stringstream ss;
    if (auto* string_data = dynamic_cast<StringSensorData*>(sensor_data)) {
        ss << "Sensor Type: " << static_cast<int>(string_data->sensor_type) << "\n"
            << "Register: " << static_cast<int>(string_data->register_) << "\n"
            << "Data Type: " << static_cast<int>(string_data->data_type) << "\n"
            << "Length: " << string_data->length << "\n"
            << "Value: ";
        for (int v : string_data->value) {
            ss << v << " ";
        }
    }
    else if (auto* int_data = dynamic_cast<IntegerSensorData*>(sensor_data)) {
        double scaler_exponent = static_cast<int>(int_data->scaler) - 5;
        double scaled_value = round(int_data->value[0] * pow(10, scaler_exponent) * 100000) / 100000;
        ss << "Sensor Type: " << static_cast<int>(int_data->sensor_type) << "\n"
            << "Register: " << static_cast<int>(int_data->register_) << "\n"
            << "Data Type: " << static_cast<int>(int_data->data_type) << "\n"
            << "Value: " << int_data->value[0] << "\n"
            << "Unit Type: " << static_cast<int>(int_data->unit_type) << "\n"
            << "Scaler: " << static_cast<int>(int_data->scaler) << "\n"
            << "Scaled value: " << scaled_value;
    }
    else {
        ss << "Unsupported SensorData type";
    }
    return ss.str();
}