#include "hmc5883l.hpp"

HMC5883L::HMC5883L() : Sensor(SensorType::HMC5883L, {
    {DataRegister::DATA_REGISTER_0, "ID"},
    {DataRegister::DATA_REGISTER_1, "Configuration_register_A"},
    {DataRegister::DATA_REGISTER_2, "Configuration_register_B"},
    {DataRegister::DATA_REGISTER_3, "Mode"},
    {DataRegister::DATA_REGISTER_4, "Status"},
    {DataRegister::DATA_REGISTER_5, "Magnetic_field_X"},
    {DataRegister::DATA_REGISTER_6, "Magnetic_field_Y"},
    {DataRegister::DATA_REGISTER_7, "Magnetic_field_Z"}
    }) {}

SensorData* HMC5883L::read_register_data(DataRegister register_) {
    switch (register_) {
        case DataRegister::DATA_REGISTER_0: {
            auto* data = new StringSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::HEX_STRING;
            data->value = value_to_list(0b10101010, DataType::HEX_STRING);
            data->length = 2;
            return data;
        }
        case DataRegister::DATA_REGISTER_1: {
            auto* data = new StringSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::BIT_STRING;
            data->value = value_to_list(0b00110101, DataType::BIT_STRING);
            data->length = 2;
            return data;
        }

        case DataRegister::DATA_REGISTER_2: {
            auto* data = new StringSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::BIT_STRING;
            data->value = value_to_list(0b11110001, DataType::BIT_STRING);
            data->length = 2;
            return data;
        }
        case DataRegister::DATA_REGISTER_3: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(0x0D0E0F10, DataType::INTEGER);
            data->unit_type = Unit::UNDEFINED;
            data->scaler = Scaler::ONE;
            return data;
        }
        case DataRegister::DATA_REGISTER_4: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(0x11121314, DataType::INTEGER);
            data->unit_type = Unit::UNDEFINED;
            data->scaler = Scaler::ONE;
            return data;
        }
        case DataRegister::DATA_REGISTER_5: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(5, DataType::INTEGER);
            data->unit_type = Unit::TESLA;
            data->scaler = Scaler::MINUS_FOUR;
            return data;
        }
        case DataRegister::DATA_REGISTER_6: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(5, DataType::INTEGER);
            data->unit_type = Unit::TESLA;
            data->scaler = Scaler::MINUS_FOUR;
            return data;
        }
        case DataRegister::DATA_REGISTER_7: {
            auto* data = new IntegerSensorData;
            data->sensor_type = sensor_type;
            data->register_ = register_;
            data->data_type = DataType::INTEGER;
            data->value = value_to_list(5, DataType::INTEGER);
            data->unit_type = Unit::TESLA;
            data->scaler = Scaler::MINUS_FOUR;
            return data;
        }
        default:
                throw std::invalid_argument("Unsupported register");
    }
}
