#include "neo6m.hpp"

NEO_6M::NEO_6M() : Sensor(SensorType::NEO_6M, {
    {DataRegister::DATA_REGISTER_0, "Time_UTC"},
    {DataRegister::DATA_REGISTER_1, "Latitude"},
    {DataRegister::DATA_REGISTER_2, "Longitude"},
    {DataRegister::DATA_REGISTER_3, "Quality"},
    {DataRegister::DATA_REGISTER_4, "Satellites_no"},
    {DataRegister::DATA_REGISTER_5, "Altitude"},
    {DataRegister::DATA_REGISTER_6, "Heading"},
    {DataRegister::DATA_REGISTER_7, "Speed_kmh"}
    }) {}

SensorData* NEO_6M::read_register_data(DataRegister register_) {
    switch (register_) {
    case DataRegister::DATA_REGISTER_0: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(112233444, DataType::INTEGER);
        data->unit_type = Unit::HHMMSS_SSS;
		data->scaler = Scaler::MINUS_THREE;
        return data;
    }
    case DataRegister::DATA_REGISTER_1: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(525200, DataType::INTEGER);
        data->unit_type = Unit::DEGREE_NORTH;
        data->scaler = Scaler::MINUS_FOUR;
        return data;
    }
    case DataRegister::DATA_REGISTER_2: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(134050, DataType::INTEGER);
        data->unit_type = Unit::DEGREE_EAST;
        data->scaler = Scaler::MINUS_FOUR;
        return data;
    }
    case DataRegister::DATA_REGISTER_3: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(1, DataType::INTEGER);
        data->unit_type = Unit::UNDEFINED;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_4: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(8, DataType::INTEGER);
        data->unit_type = Unit::UNDEFINED;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_5: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(34, DataType::INTEGER);
        data->unit_type = Unit::METER;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_6: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(180, DataType::INTEGER);
        data->unit_type = Unit::DEGREE;
        data->scaler = Scaler::ZERO;
        return data;
    }
    case DataRegister::DATA_REGISTER_7: {
        auto* data = new IntegerSensorData;
        data->sensor_type = sensor_type;
        data->register_ = register_;
        data->data_type = DataType::INTEGER;
        data->value = value_to_list(60, DataType::INTEGER);
        data->unit_type = Unit::KILOMETER_PER_HOUR;
        data->scaler = Scaler::ZERO;
        return data;
    }
    default:
        throw std::invalid_argument("Unsupported register");
    }
}

