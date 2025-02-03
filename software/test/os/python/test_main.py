import unittest
import struct
from data import (
    DlmsDataUnit, SensorType, DataRegister, DlmsDataType, DlmsPayload, DlmsObject, 
    CosemData, CosemScalerUnit, CosemRegister, create_logical_name, 
    parse_logical_name, cosem_parse_data, cosem_parse_register, 
    cosem_parse_scaler_unit, cosem_get_scaled_value, print_data_with_type, 
    print_value_with_type_and_source, print_scalable_value_with_type_and_source, print_scaled_value, 
    create_dlms_object, create_cosem_data, create_cosem_register,
    INA3221, MPU6050
)

class TestMain(unittest.TestCase):

    def test_create_logical_name(self):
        mpu6050 = MPU6050()
        logical_name = create_logical_name(mpu6050, DataRegister.DATA_REGISTER_3)
        self.assertEqual(logical_name, "0203")

    def test_parse_logical_name(self):
        sensor_name, register_name = parse_logical_name("0203")
        self.assertEqual(sensor_name, "MPU6050")
        self.assertEqual(register_name, "DATA_REGISTER_3")

    def test_create_dlms_object(self):
        payload = struct.pack('h', 1234)  
        dlms_obj = create_dlms_object(DlmsDataType.LONG, len(payload), payload)
        self.assertEqual(dlms_obj.type, DlmsDataType.LONG)
        self.assertEqual(dlms_obj.size, 2)
        self.assertEqual(dlms_obj.payload.LONG, 1234)

    def test_create_cosem_data(self):
        mpu6050 = MPU6050()
        logical_name = create_logical_name(mpu6050, DataRegister.DATA_REGISTER_3)
        payload = struct.pack('h', 1234)  
        dlms_obj = create_dlms_object(DlmsDataType.LONG, len(payload), payload)
        cosem_data = create_cosem_data(logical_name, dlms_obj)
        self.assertEqual(cosem_data.logical_name, "0203")
        self.assertEqual(cosem_data.value.payload.LONG, 1234)

    def test_create_cosem_register(self):
        mpu6050 = MPU6050()
        logical_name = create_logical_name(mpu6050, DataRegister.DATA_REGISTER_3)
        payload = struct.pack('h', 1234)  # Example payload for LONG type
        dlms_obj = create_dlms_object(DlmsDataType.LONG, len(payload), payload)
        scaler_unit = CosemScalerUnit(scaler=1, unit=2)
        cosem_register = create_cosem_register(logical_name, dlms_obj, scaler_unit)
        self.assertEqual(cosem_register.logical_name, "0203")
        self.assertEqual(cosem_register.value.payload.LONG, 1234)
        self.assertEqual(cosem_register.scaler_unit.scaler, 1)
        self.assertEqual(cosem_register.scaler_unit.unit, 2)

    def test_print_scaled_value(self):
        mpu6050 = MPU6050()
        logical_name = create_logical_name(mpu6050, DataRegister.DATA_REGISTER_3)
        payload = struct.pack('h', 1234)  # Example payload for LONG type
        dlms_obj = create_dlms_object(DlmsDataType.LONG, len(payload), payload)
        scaler_unit = CosemScalerUnit(scaler=1, unit=2)
        cosem_register = create_cosem_register(logical_name, dlms_obj, scaler_unit)
        scaled_value = cosem_get_scaled_value(cosem_register)
        self.assertAlmostEqual(scaled_value, 12340.0)  # 1234 * 10^1

    def test_dlms_data_unit(self):
            unit = DlmsDataUnit.from_id(1)
            self.assertEqual(unit.get_full_name(), "Volt")
            self.assertEqual(unit.get_literal_unit(), "V")

if __name__ == "__main__":
    unittest.main()