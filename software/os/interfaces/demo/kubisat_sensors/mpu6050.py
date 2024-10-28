from .data_types import DataType, IntegerSensorData, Scaler, Sensor, SensorType, DataRegister, SensorData, StringSensorData, Unit, value_to_list

class MPU6050(Sensor):
    def __init__(self):
        super().__init__(SensorType.MPU6050, {
            DataRegister.DATA_REGISTER_0: "ID",
            DataRegister.DATA_REGISTER_1: "Sample_rate_div",
            DataRegister.DATA_REGISTER_2: "Gyro_full_scale_range",
            DataRegister.DATA_REGISTER_3: "Accel_full_scale_range",
            DataRegister.DATA_REGISTER_4: "Power_management_A",
            DataRegister.DATA_REGISTER_5: "Power_management_B",
            DataRegister.DATA_REGISTER_6: "Accel_X",
            DataRegister.DATA_REGISTER_7: "Accel_Y",
            DataRegister.DATA_REGISTER_8: "Accel_Z",
            DataRegister.DATA_REGISTER_9: "Gyro_X",
            DataRegister.DATA_REGISTER_10: "Gyro_Y",
            DataRegister.DATA_REGISTER_11: "Gyro_Z"
        })

