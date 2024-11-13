#include "mpu6050.hpp"

MPU6050::MPU6050() : Sensor(SensorType::MPU6050, {
    {DataRegister::DATA_REGISTER_0, "ID"},
    {DataRegister::DATA_REGISTER_1, "Sample_rate_div"},
    {DataRegister::DATA_REGISTER_2, "Gyro_full_scale_range"},
    {DataRegister::DATA_REGISTER_3, "Accel_full_scale_range"},
    {DataRegister::DATA_REGISTER_4, "Power_management_A"},
    {DataRegister::DATA_REGISTER_5, "Power_management_B"},
    {DataRegister::DATA_REGISTER_6, "Accel_X"},
    {DataRegister::DATA_REGISTER_7, "Accel_Y"},
    {DataRegister::DATA_REGISTER_8, "Accel_Z"},
    {DataRegister::DATA_REGISTER_9, "Gyro_X"},
    {DataRegister::DATA_REGISTER_10, "Gyro_Y"},
    {DataRegister::DATA_REGISTER_11, "Gyro_Z"}
    }) {}

SensorData* MPU6050::read_register_data(DataRegister register_) {
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
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x05060708, DataType::INTEGER);
        data->unit_type = Unit::UNDEFINED;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_2: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x090A0B0C, DataType::INTEGER);
        data->unit_type = Unit::DEGREE_PER_SECOND;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_3: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x0D0E0F10, DataType::INTEGER);
        data->unit_type = Unit::METER;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_4: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x11121314, DataType::INTEGER);
        data->unit_type = Unit::UNDEFINED;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_5: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x15161718, DataType::INTEGER);
        data->unit_type = Unit::UNDEFINED;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_6: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x191A1B1C, DataType::INTEGER);
        data->unit_type = Unit::METER;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_7: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x1D1E1F20, DataType::INTEGER);
        data->unit_type = Unit::METER;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_8: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x21222324, DataType::INTEGER);
        data->unit_type = Unit::METER;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_9: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x25262728, DataType::INTEGER);
        data->unit_type = Unit::DEGREE_PER_SECOND;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_10: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x292A2B2C, DataType::INTEGER);
        data->unit_type = Unit::DEGREE_PER_SECOND;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_11: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(0x2D2E2F30, DataType::INTEGER);
        data->unit_type = Unit::DEGREE_PER_SECOND;
        data->scaler = Scaler::ZERO;
        return data;
    }
    default:
        throw std::invalid_argument("Unsupported register");
    }
}

