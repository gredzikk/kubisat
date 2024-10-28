import unittest
from kubisat_sensors.data_types import (
    DataRegister, SensorType, read_sensor_data, deserialize_sensor_data, pretty_print_sensor_data, get_sensor_class,
    IntegerSensorData, StringSensorData, DataType, Unit, Scaler, serialize_sensor_data
)

class TestDataTypes(unittest.TestCase):
    def test_serialize_deserialize(self):
        sensor_data = IntegerSensorData(
            sensor_type=SensorType.BMP280,
            register=DataRegister.DATA_REGISTER_3,
            data_type=DataType.INTEGER,
            value=[0, 0, 9, 220],
            unit_type=Unit.DEGREE_CELSIUS,
            scaler=Scaler.MINUS_TWO
        )
        serialized = serialize_sensor_data(sensor_data)
        deserialized = deserialize_sensor_data(serialized)
        self.assertEqual(sensor_data, deserialized)

    def test_pretty_print(self):
        sensor_data = IntegerSensorData(
            sensor_type=SensorType.BMP280,
            register=DataRegister.DATA_REGISTER_3,
            data_type=DataType.INTEGER,
            value=[0, 0, 9, 220],
            unit_type=Unit.DEGREE_CELSIUS,
            scaler=Scaler.MINUS_TWO
        )
        pretty_data = pretty_print_sensor_data(sensor_data)
        self.assertIn("Sensor Type: BMP280", pretty_data)
        self.assertIn("Register: DATA_REGISTER_3", pretty_data)
        self.assertIn("Data Type: INTEGER", pretty_data)
        self.assertIn("Value: 2524", pretty_data)
        self.assertIn("Unit Type: DEGREE_CELSIUS", pretty_data)
        self.assertIn("Scaler: Scaler.MINUS_TWO", pretty_data)
        self.assertIn("Scaled value: 25.24", pretty_data)

if __name__ == '__main__':
    unittest.main()