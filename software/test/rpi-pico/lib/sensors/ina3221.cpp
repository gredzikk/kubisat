#include "ina3221.hpp"

INA3221::INA3221() : Sensor(SensorType::INA3221, {
    {DataRegister::DATA_REGISTER_0, "Manufacturer_ID"},
    {DataRegister::DATA_REGISTER_1, "Die_ID"},
    {DataRegister::DATA_REGISTER_2, "Configuration"},
    {DataRegister::DATA_REGISTER_3, "Voltage_1"},
    {DataRegister::DATA_REGISTER_4, "Voltage_2"},
    {DataRegister::DATA_REGISTER_5, "Voltage_3"},
    {DataRegister::DATA_REGISTER_6, "Current_1"},
    {DataRegister::DATA_REGISTER_7, "Current_2"},
    {DataRegister::DATA_REGISTER_8, "Current_3"}
    }) {}

SensorData* INA3221::read_register_data(DataRegister register_) {
    switch (register_) {
    case DataRegister::DATA_REGISTER_0: {
        auto* data = new StringSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::HEX_STRING;
        data->value = value_to_list(0x01020304, DataType::HEX_STRING);
        data->length = 4;
        return data;
    }
    case DataRegister::DATA_REGISTER_1: {
        auto* data = new StringSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::HEX_STRING;
        data->value = value_to_list(0x05060708, DataType::HEX_STRING);
        data->length = 4;
        return data;
    }
    case DataRegister::DATA_REGISTER_2: {
        auto* data = new StringSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::BIT_STRING;
        data->value = value_to_list(0b00001001000010100000101100001100, DataType::BIT_STRING);
        data->length = 4;
        return data;
    }
    case DataRegister::DATA_REGISTER_3: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x0D0E0F10, DataType::INTEGER);
        data->unit_type = Unit::VOLT;
        data->scaler = Scaler::ONE;
        return data;
    }
    case DataRegister::DATA_REGISTER_4: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x11121314, DataType::INTEGER);
        data->unit_type = Unit::VOLT;
        data->scaler = Scaler::ONE;
        return data;
    }
    case DataRegister::DATA_REGISTER_5: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x15161718, DataType::INTEGER);
        data->unit_type = Unit::VOLT;
        data->scaler = Scaler::ONE;
        return data;
    }
    case DataRegister::DATA_REGISTER_6: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x191A1B1C, DataType::INTEGER);
        data->unit_type = Unit::AMPERE;
        data->scaler = Scaler::ONE;
        return data;
    }
    case DataRegister::DATA_REGISTER_7: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x1D1E1F20, DataType::INTEGER);
        data->unit_type = Unit::AMPERE;
        data->scaler = Scaler::ONE;
        return data;
    }
    case DataRegister::DATA_REGISTER_8: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x21222324, DataType::INTEGER);
        data->unit_type = Unit::AMPERE;
        data->scaler = Scaler::ONE;
        return data;
    }
    default:
        throw std::invalid_argument("Unsupported register");
    }
}
