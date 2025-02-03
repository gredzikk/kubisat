#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <map>
#include <cmath>
#include <bitset>
#include "data_types.hpp"
#include "bmp280.hpp"
#include "hmc5883l.hpp"
#include "ina3221.hpp"
#include "mpu6050.hpp"
#include "neo6m.hpp"

std::string sensor_type_to_string(SensorType sensor_type) {
    switch (sensor_type) {
    case SensorType::RESERVED: return "RESERVED";
    case SensorType::HMC5883L: return "HMC5883L";
    case SensorType::INA3221: return "INA3221";
    case SensorType::BMP280: return "BMP280";
    case SensorType::MPU6050: return "MPU6050";
    case SensorType::NEO_6M: return "NEO_6M";
    default: return "UNKNOWN";
    }
}

std::string data_register_to_string(DataRegister register_) {
    switch (register_) {
    case DataRegister::DATA_REGISTER_0: return "DATA_REGISTER_0";
    case DataRegister::DATA_REGISTER_1: return "DATA_REGISTER_1";
    case DataRegister::DATA_REGISTER_2: return "DATA_REGISTER_2";
    case DataRegister::DATA_REGISTER_3: return "DATA_REGISTER_3";
    case DataRegister::DATA_REGISTER_4: return "DATA_REGISTER_4";
    case DataRegister::DATA_REGISTER_5: return "DATA_REGISTER_5";
    case DataRegister::DATA_REGISTER_6: return "DATA_REGISTER_6";
    case DataRegister::DATA_REGISTER_7: return "DATA_REGISTER_7";
    case DataRegister::DATA_REGISTER_8: return "DATA_REGISTER_8";
    case DataRegister::DATA_REGISTER_9: return "DATA_REGISTER_9";
    case DataRegister::DATA_REGISTER_10: return "DATA_REGISTER_10";
    case DataRegister::DATA_REGISTER_11: return "DATA_REGISTER_11";
    case DataRegister::DATA_REGISTER_12: return "DATA_REGISTER_12";
    default: return "UNKNOWN";
    }
}

std::string data_type_to_string(DataType data_type) {
    switch (data_type) {
    case DataType::UNDEFINED: return "UNDEFINED";
    case DataType::INTEGER: return "INTEGER";
    case DataType::HEX_STRING: return "HEX_STRING";
    case DataType::BIT_STRING: return "BIT_STRING";
    case DataType::ENUM: return "ENUM";
    default: return "UNKNOWN";
    }
}

std::string unit_to_string(Unit unit_type) {
    switch (unit_type) {
    case Unit::UNDEFINED: return "UNDEFINED";
    case Unit::TESLA: return "TESLA";
    case Unit::DEGREE_PER_SECOND: return "DEGREE_PER_SECOND";
    case Unit::AMPERE: return "AMPERE";
    case Unit::VOLT: return "VOLT";
    case Unit::DEGREE_CELSIUS: return "DEGREE_CELSIUS";
    case Unit::PASCAL: return "PASCAL";
    case Unit::HHMMSS_SSS: return "HHMMSS_SSS";
    case Unit::DEGREE_NORTH: return "DEGREE_NORTH";
    case Unit::DEGREE_SOUTH: return "DEGREE_SOUTH";
    case Unit::DEGREE_EAST: return "DEGREE_EAST";
    case Unit::DEGREE_WEST: return "DEGREE_WEST";
    case Unit::ENUM_INTEGER: return "ENUM_INTEGER";
    case Unit::METER: return "METER";
    case Unit::DEGREE: return "DEGREE";
    case Unit::KILOMETER_PER_HOUR: return "KILOMETER_PER_HOUR";
    default: return "UNKNOWN";
    }
}

std::string scaler_to_string(Scaler scaler) {
    switch (scaler) {
    case Scaler::MINUS_FIVE: return "MINUS_FIVE";
    case Scaler::MINUS_FOUR: return "MINUS_FOUR";
    case Scaler::MINUS_THREE: return "MINUS_THREE";
    case Scaler::MINUS_TWO: return "MINUS_TWO";
    case Scaler::MINUS_ONE: return "MINUS_ONE";
    case Scaler::ZERO: return "ZERO";
    case Scaler::ONE: return "ONE";
    case Scaler::TWO: return "TWO";
    case Scaler::THREE: return "THREE";
    case Scaler::FOUR: return "FOUR";
    case Scaler::FIVE: return "FIVE";
    default: return "UNKNOWN";
    }
}


std::vector<int> value_to_list(int value, DataType data_type) {
    std::cout << "========================================================================" << std::endl;
	std::cout << "Original value: " << value << std::endl;
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
	
	std::cout << "Value to list: "; 
	for (int v : result) {
		std::cout << v; 
	}
    std::cout << std::endl;

    return result;
}

std::string create_logical_name(Sensor* sensor, DataRegister register_) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sensor->sensor_type)
        << std::setw(2) << std::setfill('0') << static_cast<int>(register_);
    return ss.str();
}

std::string serialize_sensor_data(SensorData* sensor_data) {
    std::cout << "========================================================================" << std::endl;
	std::cout << "Begin serializing data: " << std::endl; 
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sensor_data->sensor_type)
        << std::setw(2) << std::setfill('0') << static_cast<int>(sensor_data->register_)
        << std::setw(2) << std::setfill('0') << static_cast<int>(sensor_data->data_type);

    if (StringSensorData* string_data = dynamic_cast<StringSensorData*>(sensor_data)) {
        ss << std::setw(2) << std::setfill('0') << string_data->length;
        for (int v : string_data->value) {
            ss << std::setw(2) << std::setfill('0') << v;
        }
    }
    else if (IntegerSensorData* int_data = dynamic_cast<IntegerSensorData*>(sensor_data)) {
        for (int v : int_data->value) {
            ss << std::setw(2) << std::setfill('0') << v;
        }
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(int_data->unit_type)
            << std::setw(2) << std::setfill('0') << static_cast<int>(int_data->scaler);
    }
    else {
        throw std::invalid_argument("Unsupported SensorData type");
    }

	std::cout << "Serialized data: " << ss.str() << std::endl;  
    return ss.str();
}

Sensor* get_sensor_class(SensorType sensor_type) {
    switch (sensor_type) {
    case SensorType::HMC5883L:
        return new HMC5883L();
	case SensorType::INA3221:
		return new INA3221();
	case SensorType::BMP280:
		return new BMP280();
	case SensorType::MPU6050:
		return new MPU6050();
	case SensorType::NEO_6M:
		return new NEO_6M();
    default:
        return nullptr;
    }
}

std::string read_sensor_data(SensorType sensor_type, DataRegister register_) {
    std::cout << "========================================================================" << std::endl;
	std::cout << "Reading sensor data: " << std::endl; 
    Sensor* sensor = get_sensor_class(sensor_type);
    if (!sensor) {
        throw std::invalid_argument("Unsupported sensor type");
    }

    auto it = sensor->registers.find(register_);
    if (it == sensor->registers.end()) {
        delete sensor;
        throw std::invalid_argument("Unsupported register for sensor type");
    }

    SensorData* sensor_data = nullptr;

    if (!sensor_data) {
        delete sensor;
        throw std::runtime_error("Failed to read sensor data");
    }

    std::string serialized_data = serialize_sensor_data(sensor_data);
    delete sensor_data; 
    delete sensor; 

    return serialized_data;
}

SensorData* deserialize_sensor_data(const std::string& serialized_data) {
	std::cout << "========================================================================" << std::endl; 
    std::cout << "Begin deserializing data: " << serialized_data << std::endl;  // Debug print

    SensorType deserialized_sensor_type = static_cast<SensorType>(std::stoi(serialized_data.substr(0, 2), nullptr, 16));
    DataRegister deserialized_register_ = static_cast<DataRegister>(std::stoi(serialized_data.substr(2, 2), nullptr, 16));
    DataType deserialized_data_type = static_cast<DataType>(std::stoi(serialized_data.substr(4, 2), nullptr, 16));

    if (deserialized_data_type == DataType::HEX_STRING) {
        int length = std::stoi(serialized_data.substr(6, 2), nullptr, 16);
        std::cout << "Length: " << length << std::endl;  // Debug print
        std::string value;
        for (size_t i = 8; i < 8 + length * 2; i += 2) {
            value += serialized_data.substr(i, 2);
        }
        StringSensorData* stringSensorData = new StringSensorData;
        stringSensorData->sensor_type = deserialized_sensor_type;
        stringSensorData->register_ = deserialized_register_;
        stringSensorData->data_type = deserialized_data_type;
        stringSensorData->value = std::vector<int>(value.begin(), value.end());
        stringSensorData->length = length;

        return stringSensorData;
    }
    else if (deserialized_data_type == DataType::BIT_STRING) {
        int length = std::stoi(serialized_data.substr(6, 2), nullptr, 16);
        std::cout << "Length: " << length << std::endl;  // Debug print
        std::string value;
        for (size_t i = 8; i < 8 + length * 2; i += 2) {
            value += std::bitset<8>(std::stoi(serialized_data.substr(i, 2), nullptr, 16)).to_string();
        }
        StringSensorData* stringSensorData = new StringSensorData;
        stringSensorData->sensor_type = deserialized_sensor_type;
        stringSensorData->register_ = deserialized_register_;
        stringSensorData->data_type = deserialized_data_type;
        stringSensorData->value = std::vector<int>(value.begin(), value.end());
        stringSensorData->length = length;

        return stringSensorData;
    }
    else {
        int value = std::stoi(serialized_data.substr(6, 8), nullptr, 16);
        std::vector<int> value_vector;
        for (size_t i = 6; i < 14; i += 2) {
            int byte_value = std::stoi(serialized_data.substr(i, 2), nullptr, 16);
            value_vector.push_back(byte_value);
        }
        Unit deserialized_unit_type = static_cast<Unit>(std::stoi(serialized_data.substr(14, 16), nullptr, 16));
        Scaler deserialized_scaler = static_cast<Scaler>(std::stoi(serialized_data.substr(16, 18), nullptr, 16));
        IntegerSensorData* integerSensorData = new IntegerSensorData;
        integerSensorData->sensor_type = deserialized_sensor_type;
        integerSensorData->register_ = deserialized_register_;
        integerSensorData->data_type = deserialized_data_type;
        integerSensorData->value = value_vector;
        integerSensorData->unit_type = deserialized_unit_type;
		integerSensorData->scaler = deserialized_scaler;

		return integerSensorData;
    };
}


void pretty_print_sensor_data(SensorData* sensor_data) {
    std::cout << "========================================================================";
    std::stringstream ss;
    if (StringSensorData* string_data = dynamic_cast<StringSensorData*>(sensor_data)) {
        ss << "Sensor Type: " << sensor_type_to_string(string_data->sensor_type) << "\n"
            << "Register: " << data_register_to_string(string_data->register_) << "\n"
            << "Data Type: " << data_type_to_string(string_data->data_type) << "\n"
            << "Length: " << string_data->length << "\n"
            << "Value: ";
        for (int v : string_data->value) {
            ss << v;
        }
    }
    else if (IntegerSensorData* int_data = dynamic_cast<IntegerSensorData*>(sensor_data)) {
        double scaler_exponent = static_cast<int>(int_data->scaler) - 5;
        int concatenated_value = 0;
        for (size_t i = 0; i < int_data->value.size(); ++i) {
            concatenated_value = (concatenated_value << 8) | int_data->value[i];
        }
        double scaled_value = concatenated_value * pow(10, scaler_exponent);
        ss << "Sensor Type: " << sensor_type_to_string(int_data->sensor_type) << "\n"
            << "Register: " << data_register_to_string(int_data->register_) << "\n"
            << "Data Type: " << data_type_to_string(int_data->data_type) << "\n"
            << "Value: " << concatenated_value << "\n"
            << "Unit Type: " << unit_to_string(int_data->unit_type) << "\n"
            << "Scaler: " << scaler_to_string(int_data->scaler) << "\n"
            << "Scaled value: " << std::setprecision(4) << scaled_value;
    }

    else {
        ss << "Unsupported SensorData type";
    }
    
    std::cout << "\n" << ss.str() << "\n\n";
}
