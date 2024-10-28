import unittest
from kubisat_sensors.hmc5883l import HMC5883L
from kubisat_sensors.data_types import DataRegister, DataType, Unit, Scaler, IntegerSensorData, StringSensorData

class TestHMC5883L(unittest.TestCase):
    def setUp(self):
        self.sensor = HMC5883L()

    def test_read_register_data(self):
        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_0)
        self.assertIsInstance(data, StringSensorData)
        self.assertEqual(data.value, [1, 2, 3, 4])

        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_5)
        self.assertIsInstance(data, IntegerSensorData)
        self.assertEqual(data.value, [0, 0, 0, 5])
        self.assertEqual(data.unit_type, Unit.TESLA)
        self.assertEqual(data.scaler, Scaler.MINUS_FOUR)

if __name__ == '__main__':
    unittest.main()