from kubisat_sensors.data_types import DataRegister, SensorType, read_sensor_data, deserialize_sensor_data, pretty_print_sensor_data, get_sensor_class

# List of all sensor types
sensor_types = [
    SensorType.HMC5883L,
    SensorType.INA3221,
    SensorType.BMP280,
    SensorType.MPU6050,
    SensorType.NEO_6M
]

# List of all registers to read from
registers = [
    DataRegister.DATA_REGISTER_0,
    DataRegister.DATA_REGISTER_1,
    DataRegister.DATA_REGISTER_2,
    DataRegister.DATA_REGISTER_3,
    DataRegister.DATA_REGISTER_4,
    DataRegister.DATA_REGISTER_5,
    DataRegister.DATA_REGISTER_6,
    DataRegister.DATA_REGISTER_7,
    DataRegister.DATA_REGISTER_8,
    DataRegister.DATA_REGISTER_9,
    DataRegister.DATA_REGISTER_10,
    DataRegister.DATA_REGISTER_11,
    DataRegister.DATA_REGISTER_12
]

# Read, serialize, deserialize, and pretty print data for each sensor and register
for sensor_type in sensor_types:
    print(f"Reading data for sensor: {sensor_type.name}")
    sensor_class = get_sensor_class(sensor_type)
    sensor = sensor_class()
    
    for register in registers:
        if register not in sensor.registers:
            print(f"Register {register.name} not supported by sensor {sensor_type.name}")
            continue
        
        serialized_data = read_sensor_data(sensor_type, register)
        print(f"Serialized Data from {register.name}: {serialized_data}")
        
        sensor_data = deserialize_sensor_data(serialized_data)
        print(f"Pretty Deserialized Data from {register.name}:")
        print(pretty_print_sensor_data(sensor_data))
        print()  # Add a newline for better readability