enum class SensorType : uint8_t {
    INA3221 = 0x01,
    MPU6050 = 0x02,
    BMP280 = 0x03,
    NEO6M = 0x04,
    SX1278 = 0x05,
    MICROSD = 0x06,
    EEPROM = 0x07,
    QMC5883L = 0x08
};

enum class ValueType : uint8_t {
    RawValue = 0x01,
    Type = 0x02,
    Size = 0x03,
    Unit = 0x04,
    Scaler = 0x05
};

enum class DataType : uint8_t {
    Structure = 0x01,
    Array = 0x02,
    Enum = 0x03,
    Bool = 0x04,
    HexString = 0x05,
    Integer = 0x06,
    Double = 0x07,
    Position = 0x08,
    Null = 0xFF
};

enum class Unit : uint8_t {
    Volt = 0x01,
    Ampere = 0x02,
    Celsius = 0x03,
    Pascal = 0x04,
    Degree = 0x05,
    Gal = 0x06, //  980 Gal = 9,8m/s^2
    Microtesla = 0x0A,
    Undefined = 0xFF
};