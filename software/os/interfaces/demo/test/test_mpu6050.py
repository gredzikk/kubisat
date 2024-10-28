import unittest
from kubisat_sensors.mpu6050 import MPU6050
from kubisat_sensors.data_types import DataRegister, DataType, Unit, Scaler, IntegerSensorData, StringSensorData

class TestMPU6050(unittest.TestCase):
    def setUp(self):
        self.sensor = MPU6050()

    def test_read_register_data(self):
        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_0)
        self.assertIsInstance(data, StringSensorData)
        self.assertEqual(data.value, [1, 2, 3, 4])

        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_6)
        self.assertIsInstance(data, IntegerSensorData)
        self.assertEqual(data.value, [0, 0, 0, 5])
        self.assertEqual(data.unit_type, Unit.DEGREE_PER_SECOND)
        self.assertEqual(data.scaler, Scaler.ONE)

if __name__ == '__main__':
    unittest.main()