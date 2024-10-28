# FILE: main.py

import struct  # Import the struct module
from data import (
    SensorType, DataRegister, DlmsDataType, DlmsPayload, DlmsObject, 
    CosemData, CosemScalerUnit, CosemRegister, create_logical_name,  
    parse_logical_name, cosem_parse_data, cosem_parse_register, 
    cosem_parse_scaler_unit, cosem_get_scaled_value, print_data_with_type, 
    print_value_with_type_and_source, print_scalable_value_with_type_and_source, print_scaled_value, 
    create_dlms_object, create_cosem_data, create_cosem_register,
    INA3221, MPU6050, serialize_object
)

def main():
    ina3221 = INA3221()
    mpu6050 = MPU6050()

    logical_name_ina3221 = create_logical_name(ina3221, DataRegister.DATA_REGISTER_0)
    print(f"Logical Name INA3221: {logical_name_ina3221}")

    # Create a logical name for MPU6050
    logical_name_mpu6050 = create_logical_name(mpu6050, DataRegister.DATA_REGISTER_3)
    print(f"Logical Name MPU6050: {logical_name_mpu6050}")

    # Parse a logical name
    sensor_name, register_name = parse_logical_name("0103")
    print(f"Parsed Logical Name: Sensor = {sensor_name}, Register = {register_name}")

    print("\nValue with type")
    payload = struct.pack('h', 1234)  # Example payload for LONG type
    dlms_obj = create_dlms_object(DlmsDataType.LONG, len(payload), payload)
    print_data_with_type(dlms_obj)

    print("\nValue with type and source")
    cosem_data = create_cosem_data(logical_name_mpu6050, dlms_obj)
    print_value_with_type_and_source(cosem_data)

    scaler_unit = CosemScalerUnit(scaler=1, unit=2)
    
    print("\nScalable value with type and source")
    cosem_register = create_cosem_register(logical_name_mpu6050, dlms_obj, scaler_unit)
    print_scalable_value_with_type_and_source(cosem_register)

    print("\nScaled value")
    print_scaled_value(cosem_register)

    print("\nSerialized Scaler Unit")
    serialized = serialize_object(scaler_unit)
    print(f"Serialized Scaler Unit: {serialized}")

    integer_payload = struct.pack('>h', 2137)
    integer_obj = create_dlms_object(DlmsDataType.LONG, len(integer_payload), integer_payload)

    boolean_payload = struct.pack('?', True)
    boolean_obj = create_dlms_object(DlmsDataType.BOOLEAN, len(boolean_payload), boolean_payload)

    hex_string_payload = bytes("siemka", 'utf-8')
    hex_string_obj = create_dlms_object(DlmsDataType.HEX_STRING, len(hex_string_payload), hex_string_payload)

    struct_payload = [integer_obj, boolean_obj, hex_string_obj]
    struct_obj = create_dlms_object(DlmsDataType.STRUCT, len(struct_payload), struct_payload)
    
    print("\nPrinting struct_obj")
    print_data_with_type(struct_obj)

    serialized_struct = serialize_object(struct_obj)
    print(f"\nserialized struct: {serialized_struct}")
if __name__ == "__main__":
    main()
    