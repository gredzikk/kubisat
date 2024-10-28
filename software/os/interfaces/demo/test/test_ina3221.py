import unittest
from kubisat_sensors.ina3221 import INA3221
from kubisat_sensors.data_types import DataRegister, DataType, Unit, Scaler, IntegerSensorData, StringSensorData

class TestINA3221(unittest.TestCase):
    def setUp(self):
        self.sensor = INA3221()

    def test_read_register_data(self):
        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_0)
        self.assertIsInstance(data, StringSensorData)
        self.assertEqual(data.value, [1, 2, 3, 4])

        data = self.sensor.read_register_data(DataRegister.DATA_REGISTER_3)
        self.assertIsInstance(data, IntegerSensorData)
        self.assertEqual(data.value, [13, 14, 15, 16])
        self.assertEqual(data.unit_type, Unit.VOLT)
        self.assertEqual(data.scaler, Scaler.ONE)

if __name__ == '__main__':
    unittest.main()