from .data_types import DataType, IntegerSensorData, Scaler, Sensor, SensorType, DataRegister, SensorData, StringSensorData, Unit, value_to_list

class NEO_6M(Sensor):
    def __init__(self):
        super().__init__(SensorType.NEO_6M, {
            DataRegister.DATA_REGISTER_0: "Time_UTC",
            DataRegister.DATA_REGISTER_1: "Latitude",
            DataRegister.DATA_REGISTER_2: "Longitude",
            DataRegister.DATA_REGISTER_3: "Quality",
            DataRegister.DATA_REGISTER_4: "Satellites_no",
            DataRegister.DATA_REGISTER_5: "Altitude",
            DataRegister.DATA_REGISTER_6: "Heading",
            DataRegister.DATA_REGISTER_7: "Speed_kmh"
        })
