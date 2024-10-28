from .data_types import DataType, IntegerSensorData, Scaler, Sensor, SensorType, DataRegister, SensorData, StringSensorData, Unit, value_to_list

class BMP280(Sensor):
    def __init__(self):
        super().__init__(SensorType.BMP280, {
            DataRegister.DATA_REGISTER_0: "ID",
            DataRegister.DATA_REGISTER_1: "Configuration",
            DataRegister.DATA_REGISTER_2: "Measurement_control",
            DataRegister.DATA_REGISTER_3: "Temperature",
            DataRegister.DATA_REGISTER_4: "Pressure"
        })

    def read_register_data(self, register: DataRegister) -> SensorData:
        if register == DataRegister.DATA_REGISTER_0:
            return StringSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.HEX_STRING,
                value=value_to_list("58", DataType.HEX_STRING), # 8 bit value 0x58
                length=1
            )    
        # bit 7-5 t_sb[2:0] normal mode standby duration 
        # bit 4-2 filter[2:0] iir filter time constant
        # bit 0 spi3w_en[0] enables 3 wire spi when set to 1
        elif register == DataRegister.DATA_REGISTER_1:
            value = value_to_list("11100000", DataType.BIT_STRING) # 4000ms standby, filter off, spi off
            length = len(value)
            return StringSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.BIT_STRING,
                value=value,
                length=length
            )
        elif register == DataRegister.DATA_REGISTER_2:
            value = value_to_list("10010011", DataType.BIT_STRING) # osrs_t and _p 8x, mode normal
            length = len(value)
            return StringSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.BIT_STRING,
                value=value,
                length=length
            )
        elif register == DataRegister.DATA_REGISTER_3:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(2524, DataType.INTEGER), # value in deg
                unit_type=Unit.DEGREE_CELSIUS,
                scaler=Scaler.MINUS_TWO
            )
        elif register == DataRegister.DATA_REGISTER_4:
            return IntegerSensorData(
                sensor_type=self.sensor_type,
                register=register,
                data_type=DataType.INTEGER,
                value=value_to_list(102500, DataType.INTEGER), # value in Pa
                unit_type=Unit.PASCAL,
                scaler=Scaler.ONE
            )
        else:
            raise ValueError(f"Unsupported register: {register}")