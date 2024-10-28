# data_types.py
from enum import Enum
import struct
from dataclasses import dataclass
from typing import List

class Unit(Enum):
    UNDEFINED = 0
    TESLA = 1
    DEGREE_PER_SECOND = 2
    AMPERE = 3
    VOLT = 4
    DEGREE_CELSIUS = 5
    PASCAL = 6
    HHMMSS_SSS = 7
    DEGREE_NORTH = 8
    DEGREE_SOUTH = 9
    DEGREE_EAST = 10
    DEGREE_WEST = 11
    ENUM_INTEGER = 12
    METER = 13
    DEGREE = 14
    KILOMETER_PER_HOUR = 15

class Scaler(Enum):
    MINUS_FIVE = 0
    MINUS_FOUR = 1
    MINUS_THREE = 2
    MINUS_TWO = 3
    MINUS_ONE = 4
    ZERO = 5
    ONE = 6
    TWO = 7
    THREE = 8
    FOUR = 9
    FIVE = 10

class DataType(Enum):
    UNDEFINED = 0
    INTEGER = 1
    HEX_STRING = 2
    BIT_STRING = 3
    ENUM = 4

class SensorType(Enum):
    RESERVED = 0
    HMC5883L = 1
    INA3221 = 2
    BMP280 = 3
    MPU6050 = 4
    NEO_6M = 5

class DataRegister(Enum):
    DATA_REGISTER_0 = 0
    DATA_REGISTER_1 = 1
    DATA_REGISTER_2 = 2
    DATA_REGISTER_3 = 3
    DATA_REGISTER_4 = 4
    DATA_REGISTER_5 = 5
    DATA_REGISTER_6 = 6
    DATA_REGISTER_7 = 7
    DATA_REGISTER_8 = 8
    DATA_REGISTER_9 = 9
    DATA_REGISTER_10 = 10
    DATA_REGISTER_11 = 11
    DATA_REGISTER_12 = 12

class Sensor:
    def __init__(self, sensor_type, registers):
        self.sensor_type = sensor_type
        self.registers = registers

@dataclass
class SensorData:
    sensor_type: SensorType
    register: DataRegister
    data_type: DataType
    value: List[int]

@dataclass
class IntegerSensorData(SensorData):
    unit_type: Unit
    scaler: Scaler

@dataclass
class StringSensorData(SensorData):
    length: int

def value_to_list(value, data_type: DataType) -> List[int]:
    if data_type == DataType.INTEGER:
        # Convert the integer to a hex string and pad it to 8 hex digits
        hex_value = f"{value:08x}"
        # Convert the hex string to a list of integers
        return [int(hex_value[i:i+2], 16) for i in range(0, len(hex_value), 2)]
    elif data_type == DataType.BIT_STRING:
        # Pad the bit string to make its length a multiple of 4
        bit_string = value.zfill((len(value) + 3) // 4 * 4)
        # Convert each 4-bit chunk to a hex digit
        return [int(bit_string[i:i+4], 2) for i in range(0, len(bit_string), 4)]
    elif data_type == DataType.HEX_STRING:
        return [int(value[i:i+2], 16) for i in range(0, len(value), 2)]
    else:
        raise ValueError(f"Unsupported data type: {data_type}")

def create_logical_name(sensor, register):
    sensor_id = sensor.sensor_type.value
    register_id = register.value
    logical_name = struct.pack('BB', sensor_id, register_id)
    return logical_name.hex()

def get_sensor_class(sensor_type: SensorType):
    if sensor_type == SensorType.HMC5883L:
        from .hmc5883l import HMC5883L
        return HMC5883L
    elif sensor_type == SensorType.INA3221:
        from .ina3221 import INA3221
        return INA3221
    elif sensor_type == SensorType.BMP280:
        from .bmp280 import BMP280
        return BMP280
    elif sensor_type == SensorType.MPU6050:
        from .mpu6050 import MPU6050
        return MPU6050
    elif sensor_type == SensorType.NEO_6M:
        from .neo6m import NEO_6M
        return NEO_6M
    else:
        raise ValueError(f"Unsupported sensor type: {sensor_type}")

def serialize_sensor_data(sensor_data: SensorData) -> str:
    sensor_type = f"{sensor_data.sensor_type.value:02x}"
    register = f"{sensor_data.register.value:02x}"
    data_type = f"{sensor_data.data_type.value:02x}"
    
    if isinstance(sensor_data, StringSensorData):
        length = f"{sensor_data.length:02x}"
        value = ''.join(f"{v:02x}" for v in sensor_data.value)
        return f"{sensor_type}{register}{data_type}{length}{value}"
    elif isinstance(sensor_data, IntegerSensorData):
        value = ''.join(f"{v:02x}" for v in sensor_data.value)
        unit_type = f"{sensor_data.unit_type.value:02x}"
        scaler = f"{sensor_data.scaler.value:02x}"  # Use .value to get the integer value of the enum
        return f"{sensor_type}{register}{data_type}{value}{unit_type}{scaler}"
    else:
        raise ValueError(f"Unsupported SensorData type: {type(sensor_data)}")

def read_sensor_data(sensor_type: SensorType, register: DataRegister) -> str:
    sensor_class = get_sensor_class(sensor_type)
    sensor = sensor_class()
    
    register_name = sensor.registers.get(register)
    if not register_name:
        raise ValueError(f"Unsupported register: {register} for sensor type: {sensor_type}")

    # Read data from the specified register
    sensor_data = sensor.read_register_data(register)
    
    # Serialize the sensor data to an octet-string
    serialized_data = serialize_sensor_data(sensor_data)
    
    return serialized_data

def deserialize_sensor_data(serialized_data: str) -> SensorData:
    print(f"Deserializing data: {serialized_data}")  # Debug print
    sensor_type = SensorType(int(serialized_data[0:2], 16))
    register = DataRegister(int(serialized_data[2:4], 16))
    data_type = DataType(int(serialized_data[4:6], 16))
    
    if data_type == DataType.HEX_STRING:
        length = int(serialized_data[6:8], 16)
        print(f"Length: {length}")  # Debug print
        value = ''.join(f"{serialized_data[i:i+2]}" for i in range(8, 8 + length * 2, 2))
        return StringSensorData(sensor_type=sensor_type, register=register, data_type=data_type, length=length, value=value)
    elif data_type == DataType.BIT_STRING:
        length = int(serialized_data[6:8], 16)
        print(f"Length: {length}")  # Debug print
        value = ''.join(f"{int(serialized_data[i:i+2], 16):04b}" for i in range(8, 8 + length * 2, 2))
        return StringSensorData(sensor_type=sensor_type, register=register, data_type=data_type, length=length, value=value)
    else:
        value = int(''.join(f"{int(serialized_data[i:i+2], 16):02x}" for i in range(6, 14, 2)), 16)
        unit_type = Unit(int(serialized_data[14:16], 16))
        scaler = Scaler(int(serialized_data[16:18], 16))
        return IntegerSensorData(sensor_type=sensor_type, register=register, data_type=data_type, value=value, unit_type=unit_type, scaler=scaler)
    
def pretty_print_sensor_data(sensor_data: SensorData) -> str:
    if isinstance(sensor_data, StringSensorData):
        return (f"Sensor Type: {sensor_data.sensor_type.name}\n"
                f"Register: {sensor_data.register.name}\n"
                f"Data Type: {sensor_data.data_type.name}\n"
                f"Length: {sensor_data.length}\n"
                f"Value: {sensor_data.value}")
    elif isinstance(sensor_data, IntegerSensorData):
        scaler_exponent = sensor_data.scaler.value - 5
        scaled_value = round(sensor_data.value * (10 ** scaler_exponent), 5)
        return (f"Sensor Type: {sensor_data.sensor_type.name}\n"
                f"Register: {sensor_data.register.name}\n"
                f"Data Type: {sensor_data.data_type.name}\n"
                f"Value: {sensor_data.value}\n"
                f"Unit Type: {sensor_data.unit_type.name}\n"
                f"Scaler: {sensor_data.scaler}\n"
                f"Scaled value: {scaled_value}")
    else:
        return "Unsupported SensorData type"