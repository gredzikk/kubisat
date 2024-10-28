from .data_types import DataType, IntegerSensorData, Scaler, Sensor, SensorType, DataRegister, SensorData, StringSensorData, Unit, value_to_list

class HMC5883L(Sensor):
    def __init__(self):
        super().__init__(SensorType.HMC5883L, {
            DataRegister.DATA_REGISTER_0: "ID",
            DataRegister.DATA_REGISTER_1: "Configuration_register_A",
            DataRegister.DATA_REGISTER_2: "Configuration_register_B",
            DataRegister.DATA_REGISTER_3: "Mode",
            DataRegister.DATA_REGISTER_4: "Status",
            DataRegister.DATA_REGISTER_5: "Magnetic_field_X",
            DataRegister.DATA_REGISTER_6: "Magnetic_field_Y",
            DataRegister.DATA_REGISTER_7: "Magnetic_field_Z"
        })

    def read_register_data(self, register: DataRegister) -> SensorData:
        if register == DataRegister.DATA_REGISTER_0:
            return StringSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.HEX_STRING,
                value=value_to_list("01020304", DataType.HEX_STRING),
                length=4
            )
        elif register == DataRegister.DATA_REGISTER_1:
            return StringSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.BIT_STRING,
                value=value_to_list("00000101000001100000011100001000", DataType.BIT_STRING),
                length=4
            )
        elif register == DataRegister.DATA_REGISTER_2:
            return StringSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.BIT_STRING,
                value=value_to_list("00001001000010100000101100001100", DataType.BIT_STRING),
                length=4
            )
        elif register == DataRegister.DATA_REGISTER_3:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x0D0E0F10, DataType.INTEGER),
                unit_type=Unit.UNDEFINED,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_4:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x11121314, DataType.INTEGER),
                unit_type=Unit.UNDEFINED,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_5:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(5, DataType.INTEGER),
                unit_type=Unit.TESLA,
                scaler=Scaler.MINUS_FOUR
            )
        elif register == DataRegister.DATA_REGISTER_6:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(5, DataType.INTEGER),
                unit_type=Unit.TESLA,
                scaler=Scaler.MINUS_FOUR
            )
        elif register == DataRegister.DATA_REGISTER_7:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(5, DataType.INTEGER),
                unit_type=Unit.TESLA,
                scaler=Scaler.MINUS_FOUR
            )
        else:
            raise ValueError(f"Unsupported register: {register}")

