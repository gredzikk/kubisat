from enum import Enum
import math
import struct

class SensorType(Enum):
    RESERVED = 0
    INA3221 = 1
    MPU6050 = 2
    QMC5883L = 3
    BMP280 = 4

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
    DATA_REGISTER_13 = 13
    DATA_REGISTER_14 = 14
    DATA_REGISTER_15 = 15
    DATA_REGISTER_16 = 16
    DATA_REGISTER_17 = 17
    DATA_REGISTER_18 = 18
    DATA_REGISTER_19 = 19
    DATA_REGISTER_20 = 20

class DlmsDataType(Enum):
    BOOLEAN = 0
    INTEGER = 1
    UNSIGNED = 2
    ENUM = 3
    LONG = 4
    DOUBLE = 5
    HEX_STRING = 6
    ARRAY = 7
    STRUCT = 8
    NULL_DATA = 9

class DlmsDataUnit(Enum):
    METER = (0, "Meter", "m")
    VOLT = (1, "Volt", "V")
    AMPERE = (2, "Ampere", "A")
    GAL = (3, "Gal", "Gal")
    TESLA = (4, "Tesla", "T")
    DEGREE_CELSIUS = (5, "Degree Celsius", "°C")
    PERCENT = (6, "Percent", "%")
    SECOND = (7, "Second", "s")
    PASCAL = (8, "Pascal", "Pa")
    METER_PER_SECOND = (9, "Meter per Second", "m/s")

    def __init__(self, id, full_name, literal_unit):
        self.id = id
        self.full_name = full_name
        self.literal_unit = literal_unit

    @classmethod
    def from_id(cls, id):
        for unit in cls:
            if unit.id == id:
                return unit
        raise ValueError(f"No DlmsDataUnit with id {id}")

    def get_full_name(self):
        return self.full_name

    def get_literal_unit(self):
        return self.literal_unit

class DlmsPayload:
    def __init__(self):
        self.HEX_STRING = None
        self.INTEGER = None
        self.UNSIGNED = None
        self.LONG = None
        self.DOUBLE = None
        self.BOOL = None
        self.OBJECT = None
        self.raw = None

class DlmsObject:
    def __init__(self, type, size, payload):
        self.type = type
        self.size = size
        self.payload = payload

class CosemData:
    def __init__(self, logical_name, value):
        self.logical_name = logical_name
        self.value = value

class CosemScalerUnit:
    def __init__(self, scaler, unit):
        self.scaler = scaler
        self.unit = unit

class CosemRegister:
    def __init__(self, logical_name, value, scaler_unit):
        self.logical_name = logical_name
        self.value = value
        self.scaler_unit = scaler_unit

class Sensor:
    def __init__(self, sensor_type, registers):
        self.sensor_type = sensor_type
        self.registers = registers

class INA3221(Sensor):
    def __init__(self):
        super().__init__(SensorType.INA3221, {
            DataRegister.DATA_REGISTER_0: "Voltage_1",
            DataRegister.DATA_REGISTER_1: "Voltage_2",
            DataRegister.DATA_REGISTER_2: "Voltage_3",
            DataRegister.DATA_REGISTER_3: "Current_1",
            DataRegister.DATA_REGISTER_4: "Current_2",
            DataRegister.DATA_REGISTER_5: "Current_3",
        })

class MPU6050(Sensor):
    def __init__(self):
        super().__init__(SensorType.MPU6050, {
            DataRegister.DATA_REGISTER_0: "Accel_X",
            DataRegister.DATA_REGISTER_1: "Accel_Y",
            DataRegister.DATA_REGISTER_2: "Accel_Z",
            DataRegister.DATA_REGISTER_3: "Gyro_X",
            DataRegister.DATA_REGISTER_4: "Gyro_Y",
            DataRegister.DATA_REGISTER_5: "Gyro_Z",
        })


def create_logical_name(sensor, register):
    sensor_id = sensor.sensor_type.value
    register_id = register.value
    logical_name = struct.pack('BB', sensor_id, register_id)
    return logical_name.hex()

def parse_logical_name(logical_name):
    sensor_id, register_id = struct.unpack('BB', bytes.fromhex(logical_name))
    return SensorType(sensor_id).name, DataRegister(register_id).name

def cosem_parse_data(obj):
    return CosemData(obj.payload.OBJECT[0].payload.HEX_STRING, obj.payload.OBJECT[1])

def cosem_parse_register(obj):
    return CosemRegister(
        obj.payload.OBJECT[0].payload.HEX_STRING,
        obj.payload.OBJECT[1],
        cosem_parse_scaler_unit(obj.payload.OBJECT[2])
    )

def cosem_parse_scaler_unit(obj):
    return CosemScalerUnit(
        obj.payload.OBJECT[0].payload.INTEGER,
        obj.payload.OBJECT[1].payload.UNSIGNED
    )

def cosem_get_scaled_value(reg):
    return reg.value.payload.LONG * math.pow(10, reg.scaler_unit.scaler)

def print_data_with_type(obj):
    if obj.type == DlmsDataType.BOOLEAN:
        print(f"Type: BOOLEAN\nValue: {bool(obj.payload.BOOL)}")
    elif obj.type == DlmsDataType.INTEGER:
        print(f"Type: INTEGER\nValue: {obj.payload.INTEGER}")
    elif obj.type == DlmsDataType.UNSIGNED:
        print(f"Type: UNSIGNED\nValue: {obj.payload.UNSIGNED}")
    elif obj.type == DlmsDataType.LONG:
        print(f"Type: LONG\nValue: {obj.payload.LONG}")
    elif obj.type == DlmsDataType.DOUBLE:
        print(f"Type: DOUBLE\nValue: {obj.payload.DOUBLE}")
    elif obj.type == DlmsDataType.HEX_STRING:
        print(f"Type: HEX_STRING\nValue: {obj.payload.HEX_STRING}")
    elif obj.type == DlmsDataType.STRUCT:
        print(f"Type: STRUCT")
        for i in range(obj.size):
            print(f"  Element {i}:")
            print_data_with_type(obj.payload.OBJECT[i])
    elif obj.type == DlmsDataType.ARRAY:
        print(f"Type: ARRAY")
        for i in range(obj.size):
            print(f"  Element {i}:")
            print_data_with_type(obj.payload.OBJECT[i])
    else:
        print("Type: Unknown")

def print_value_with_type_and_source(data):
    print(f"Logical Name: {data.logical_name}")
    print_data_with_type(data.value)

def print_scalable_value_with_type_and_source(reg):
    print(f"Logical Name: {reg.logical_name}")
    if reg.value.type == DlmsDataType.INTEGER:
        print(f"Value: {reg.value.payload.INTEGER}")
    elif reg.value.type == DlmsDataType.LONG:
        print(f"Value: {reg.value.payload.LONG}")
    print(f"  Scaler: {reg.scaler_unit.scaler}")
    print(f"  Unit: {DlmsDataUnit.from_id(reg.scaler_unit.unit).get_full_name()} - {reg.scaler_unit.unit}")
    
def print_scaled_value(reg):
    scaled_value = cosem_get_scaled_value(reg)
    unit = DlmsDataUnit.from_id(reg.scaler_unit.unit).get_literal_unit()
    print(f"Scaled Value: {scaled_value} {unit}")

def data_type_len(type):
    if type in {DlmsDataType.BOOLEAN, DlmsDataType.INTEGER, DlmsDataType.UNSIGNED, DlmsDataType.ENUM}:
        return 1
    elif type == DlmsDataType.LONG:
        return 2
    elif type == DlmsDataType.DOUBLE:
        return 4
    else:
        return 0

def reverse_bytes(inp, len):
    return inp[::-1]

def dlms_parse_payload(obj, ptr):
    result = DlmsPayload()
    size = 0
    if obj.type == DlmsDataType.STRUCT:
        result.raw = [DlmsObject(None, None, None) for _ in range(obj.size)]
        for i in range(obj.size):
            size += dlms_parse_object(result.raw[i], ptr[size:])
    elif obj.type == DlmsDataType.ARRAY:
        result.raw = ptr
    elif obj.type == DlmsDataType.HEX_STRING:
        size = obj.size
        result.raw = ptr[:size]
    elif obj.type in {DlmsDataType.BOOLEAN, DlmsDataType.DOUBLE, DlmsDataType.INTEGER, DlmsDataType.LONG, DlmsDataType.UNSIGNED, DlmsDataType.ENUM}:
        size = obj.size
        result.raw = reverse_bytes(ptr[:size], size)
    elif obj.type == DlmsDataType.NULL_DATA:
        size = 0
    obj.payload = result
    return size

def get_size_from_pdu(size, type_bytes, pdu):
    type = DlmsDataType(pdu[0])
    size[0] = data_type_len(type)
    fixed_size = size[0] or not type
    type_bytes[0] = 1
    if not fixed_size:
        size[0] = pdu[1]
        type_bytes[0] = 2

def dlms_parse_object(dest, pdu):
    type_bytes_local = 0
    dest.type = DlmsDataType(pdu[0])
    get_size_from_pdu([dest.size], [type_bytes_local], pdu)
    payload_size = dlms_parse_payload(dest, pdu[type_bytes_local:])
    return payload_size + type_bytes_local

def dlms_free_object(obj):
    if obj.type == DlmsDataType.ARRAY:
        return
    elif obj.type == DlmsDataType.STRUCT:
        for i in range(obj.size):
            dlms_free_object(obj.payload.OBJECT[i])
    if obj.size > 0:
        obj.payload.raw = None

def create_dlms_object(type, size, payload):
    obj = DlmsObject(type, size, DlmsPayload())
    match type:
        case DlmsDataType.HEX_STRING:
            obj.payload.HEX_STRING = payload[:size]
        case DlmsDataType.INTEGER:
            obj.payload.INTEGER = payload[:size]
        case DlmsDataType.UNSIGNED:
            obj.payload.UNSIGNED = payload[:size]
        case DlmsDataType.LONG:
            obj.payload.LONG = struct.unpack('>h', payload[:size])[0]
        case DlmsDataType.BOOLEAN:
            obj.payload.BOOL = struct.unpack('?', payload[:size])[0]
        case DlmsDataType.DOUBLE:
            obj.payload.DOUBLE = struct.unpack('>d', payload[:size])[0]
        case DlmsDataType.STRUCT:
            obj.payload.OBJECT = payload[:size]
        case DlmsDataType.ARRAY:
            obj.payload.OBJECT = payload[:size]
        case DlmsDataType.ENUM:
            obj.payload.UNSIGNED = payload[:size]
        case DlmsDataType.NULL_DATA:
            obj.payload.raw = None
    return obj

def create_cosem_data(logical_name, value):
    return CosemData(logical_name, value)

def create_cosem_register(logical_name, value, scaler_unit):
    return CosemRegister(logical_name, value, scaler_unit)

def serialize_object(obj):
    if isinstance(obj, DlmsObject):
        if obj.type == DlmsDataType.STRUCT:
            serialized = f"{DlmsDataType.STRUCT.value:02X}{obj.size:02X}"
            for element in obj.payload.OBJECT:
                serialized += serialize_object(element)
            return serialized
        elif obj.type == DlmsDataType.INTEGER:
            return f"{DlmsDataType.INTEGER.value:02X}{obj.payload.INTEGER[0]:02X}"
        elif obj.type == DlmsDataType.LONG:
            return f"{DlmsDataType.LONG.value:02X}{obj.payload.LONG:04X}"
        elif obj.type == DlmsDataType.BOOLEAN:
            return f"{DlmsDataType.BOOLEAN.value:02X}{int(obj.payload.BOOL):02X}"
        elif obj.type == DlmsDataType.HEX_STRING:
            return f"{DlmsDataType.HEX_STRING.value:02X}{obj.payload.HEX_STRING.hex()}"
        elif obj.type == DlmsDataType.ENUM:
            return f"{DlmsDataType.ENUM.value:02X}{obj.payload.UNSIGNED[0]:02X}"
    elif isinstance(obj, CosemScalerUnit):
        serialized = f"{DlmsDataType.STRUCT.value:02X}02"
        serialized += f"{DlmsDataType.INTEGER.value:02X}{obj.scaler:02X}"
        serialized += f"{DlmsDataType.ENUM.value:02X}{obj.unit:02X}"
        return serialized
    return ""
