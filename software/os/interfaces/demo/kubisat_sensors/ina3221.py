from .data_types import DataType, IntegerSensorData, Scaler, Sensor, SensorType, DataRegister, SensorData, StringSensorData, Unit, value_to_list

class INA3221(Sensor):
    def __init__(self):
        super().__init__(SensorType.INA3221, {
            DataRegister.DATA_REGISTER_0: "Manufacturer_ID",
            DataRegister.DATA_REGISTER_1: "Die_ID",
            DataRegister.DATA_REGISTER_2: "Configuration",
            DataRegister.DATA_REGISTER_3: "Voltage_1", 
            DataRegister.DATA_REGISTER_4: "Voltage_2", 
            DataRegister.DATA_REGISTER_5: "Voltage_3", 
            DataRegister.DATA_REGISTER_6: "Current_1",            
            DataRegister.DATA_REGISTER_7: "Current_2",
            DataRegister.DATA_REGISTER_8: "Current_3"
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
                data_type=DataType.HEX_STRING,
                value=value_to_list("05060708", DataType.HEX_STRING),
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
                unit_type=Unit.VOLT,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_4:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x11121314, DataType.INTEGER),
                unit_type=Unit.VOLT,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_5:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x15161718, DataType.INTEGER),
                unit_type=Unit.VOLT,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_6:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x191A1B1C, DataType.INTEGER),
                unit_type=Unit.AMPERE,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_7:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x1D1E1F20, DataType.INTEGER),
                unit_type=Unit.AMPERE,
                scaler=Scaler.ONE
            )
        elif register == DataRegister.DATA_REGISTER_8:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(0x21222324, DataType.INTEGER),
                unit_type=Unit.AMPERE,
                scaler=Scaler.ONE
            )
        else:
            raise ValueError(f"Unsupported register: {register}")
