import unittest
from unittest.mock import patch, MagicMock
from kubisat_sensors.data_types import DataRegister, SensorType, read_sensor_data, deserialize_sensor_data, pretty_print_sensor_data, get_sensor_class

class TestMain(unittest.TestCase):
    @patch('kubsat_sensors.data_types.read_sensor_data')
    @patch('kubsat_sensors.data_types.deserialize_sensor_data')
    @patch('kubsat_sensors.data_types.pretty_print_sensor_data')
    def test_read_all_registers(self, mock_pretty_print, mock_deserialize, mock_read):
        # Mock the return values
        mock_read.return_value = "mock_serialized_data"
        mock_deserialize.return_value = MagicMock()
        mock_pretty_print.return_value = "mock_pretty_data"

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

        for sensor_type in sensor_types:
            sensor_class = get_sensor_class(sensor_type)
            sensor = sensor_class()
            
            for register in registers:
                if register not in sensor.registers:
                    continue
                
                serialized_data = read_sensor_data(sensor_type, register)
                self.assertEqual(serialized_data, "mock_serialized_data")
                
                sensor_data = deserialize_sensor_data(serialized_data)
                self.assertIsNotNone(sensor_data)
                
                pretty_data = pretty_print_sensor_data(sensor_data)
                self.assertEqual(pretty_data, "mock_pretty_data")

if __name__ == '__main__':
    unittest.main()