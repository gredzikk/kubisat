#include "bmp280.hpp"

BMP280::BMP280() : Sensor(SensorType::BMP280, {
    {DataRegister::DATA_REGISTER_0, "ID"},
    {DataRegister::DATA_REGISTER_1, "Configuration"},
    {DataRegister::DATA_REGISTER_2, "Measurement_control"},
    {DataRegister::DATA_REGISTER_3, "Temperature"},
    {DataRegister::DATA_REGISTER_4, "Pressure"}
    }) {}

SensorData* BMP280::read_register_data(DataRegister register_) {
    switch (register_) {
        case DataRegister::DATA_REGISTER_0: {
            auto* data = new StringSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::HEX_STRING;
            data->value = value_to_list(0x58, DataType::HEX_STRING); // 8 bit value 0x58
            data->length = 1;
            return data;
        }
        case DataRegister::DATA_REGISTER_1: {
            std::vector<int> value = value_to_list(0b11100000, DataType::BIT_STRING); // 4000ms standby, filter off, spi off
            auto* data = new StringSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::BIT_STRING;
            data->value = value;
            data->length = value.size()/4;
            return data;
        }
        case DataRegister::DATA_REGISTER_2: {
            std::vector<int> value = value_to_list(0b10010011, DataType::BIT_STRING); // osrs_t and _p 8x, mode normal
            auto* data = new StringSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::BIT_STRING;
            data->value = value;
            data->length = 0;
            printf("%d", value.size());
            return data;
        }
        case DataRegister::DATA_REGISTER_3: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(2524, DataType::INTEGER); // value in deg
            data->unit_type = Unit::DEGREE_CELSIUS;
            data->scaler = Scaler::MINUS_TWO;
            return data;
        }
        case DataRegister::DATA_REGISTER_4: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(102500, DataType::INTEGER); // value in Pa
            data->unit_type = Unit::PASCAL;
            data->scaler = Scaler::MINUS_TWO;
            return data;
        }
        // default:
        //     throw std::invalid_argument("Unsupported register");
    }
}