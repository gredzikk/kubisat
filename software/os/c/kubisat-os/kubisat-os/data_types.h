#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <cstddef>

typedef uint8_t* logical_name[5];

typedef enum class {
    INA3221 = 0,
    MPU6050,
    QMC5883L,
    BMP280,
}sensor_type;

typedef enum class {
    DATA_REGISTER_0 = 0,
    DATA_REGISTER_1,
    DATA_REGISTER_2,
    DATA_REGISTER_3,
    DATA_REGISTER_4,
    DATA_REGISTER_5,
    DATA_REGISTER_6,
    DATA_REGISTER_7,
    DATA_REGISTER_8,
    DATA_REGISTER_9,
    DATA_REGISTER_10,
    DATA_REGISTER_11,
    DATA_REGISTER_12,
    DATA_REGISTER_13,
    DATA_REGISTER_14,
    DATA_REGISTER_15,
    DATA_REGISTER_16,
    DATA_REGISTER_17,
    DATA_REGISTER_18,
    DATA_REGISTER_19,
    DATA_REGISTER_20
}DataRegister;

enum class dlms_data_type {
    BOOLEAN,
    INTEGER,
    UNSIGNED,
    ENUM,
    LONG,
    DOUBLE,
    HEX_STRING,
    ARRAY,
    STRUCT,
    NULL_DATA
}
;

typedef enum class {
    METER,
    VOLT,
    AMPERE,
    GAL,
    TESLA,
    DEGREE_CELSIUS,
    PERCENT,
    SECOND,
    PASCAL,
    METER_PER_SECOND,
}
dlms_data_unit;

typedef union {
    uint8_t* HEX_STRING;
    int8_t* INTEGER;
    uint8_t* UNSIGNED;
    int16_t* LONG;
    double* DOUBLE;
    uint8_t* BOOL;
    struct dlms_object* OBJECT;
    void* raw;
}
dlms_payload;

typedef struct dlms_object {
    dlms_data_type type;
    std::size_t size;
    dlms_payload payload;
}
dlms_object;

typedef struct cosem_data {
    uint8_t* logical_name;
    dlms_object value;
}
cosem_data;

typedef struct cosem_scaler_unit {
    int8_t scaler;
    uint8_t unit;
}
cosem_scaler_unit;

typedef struct cosem_register {
    uint8_t* logical_name;
    dlms_object value;
    cosem_scaler_unit scaler_unit;
}
cosem_register;

cosem_data cosem_parse_data(const dlms_object& obj);
cosem_register cosem_parse_register(const dlms_object& obj);
cosem_scaler_unit cosem_parse_scaler_unit(const dlms_object& obj);
double cosem_get_scaled_value(const cosem_register& reg);
void dlms_free_object(dlms_object& obj);

void print_cosem_data(const cosem_data& data);
void print_cosem_register(const cosem_register& reg);
void print_scaled_value(const cosem_register& reg);

dlms_object create_dlms_object(dlms_data_type type, size_t size, void* payload);
cosem_register create_cosem_register(uint8_t* logical_name, dlms_object value, cosem_scaler_unit scaler_unit);
cosem_data create_cosem_data(uint8_t* logical_name, dlms_object value);
std::size_t dlms_parse_object(dlms_object* dest, char* pdu);
std::unique_ptr<uint8_t[]> create_logical_name(sensor_type sensor, DataRegister data);

#endif // DATA_TYPES_HPP
