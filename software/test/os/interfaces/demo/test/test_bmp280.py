import unittest
from kubisat_sensors.bmp280 import BMP280
from kubisat_sensors.data_types import DataRegister, DataType, Unit, Scaler, IntegerSensorData, StringSensorData

class TestBMP280(unittest.TestCase):
    def setUp(self):
        self.sensor = BMP280()

    def test_read_register_data(self):
        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_0)
        self.assertIsInstance(data, StringSensorData)
        self.assertEqual(data.value, [88])

        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_3)
        self.assertIsInstance(data, IntegerSensorData)
        self.assertEqual(data.value, [0, 0, 9, 220])
        self.assertEqual(data.unit_type, Unit.DEGREE_CELSIUS)
        self.assertEqual(data.scaler, Scaler.MINUS_TWO)

if __name__ == '__main__':
    unittest.main()