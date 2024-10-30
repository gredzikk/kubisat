#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <iostream>
#include <vector>
#include <string>
#include <map>

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

class SensorData {
public:
    SensorType sensor_type;
    DataRegister register_;
    DataType data_type;
    std::vector<int> value;

    virtual ~SensorData() = default;
};

class IntegerSensorData : public SensorData {
public:
    Unit unit_type;
    Scaler scaler;
};

class StringSensorData : public SensorData {
public:
    size_t length;
};

class Sensor {
public:
    Sensor(SensorType sensor_type, std::map<DataRegister, std::string> registers)
        : sensor_type(sensor_type), registers(registers) {}
    virtual ~Sensor() {}
    virtual SensorData* read_register_data(DataRegister register_) = 0;

    SensorType sensor_type;
    std::map<DataRegister, std::string> registers;
};

std::vector<int> value_to_list(int value, DataType data_type);
std::string create_logical_name(Sensor* sensor, DataRegister register_);
Sensor* get_sensor_class(SensorType sensor_type);
std::string serialize_sensor_data(SensorData* sensor_data);
std::string read_sensor_data(SensorType sensor_type, DataRegister register_);
SensorData* deserialize_sensor_data(const std::string& serialized_data);
void pretty_print_sensor_data(SensorData* sensor_data);

#endif // DATA_TYPES_H