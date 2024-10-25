#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct cosem_scaler_unit {
    int8_t scaler;
    uint8_t unit;
};

struct cosem_data {
    unsigned char* logical_name;
    struct dlms_object value;
};

struct cosem_register {
    unsigned char* logical_name;
    struct dlms_object value;
    struct cosem_scaler_unit scaler_unit;
};

/**
 * Calculate the real value of a register by scaling it
 * according to the scaler value in the scaler_unit struct
 * 
 * @return double v * 10 ^ s (where v is value and s is scaler)
 */
double cosem_get_scaled_value(struct cosem_register);

struct cosem_data cosem_parse_data(struct dlms_object);
struct cosem_scaler_unit cosem_parse_scaler_unit(struct dlms_object);
struct cosem_register cosem_parse_register(struct dlms_object);

struct dlms_object;

enum dlms_data_type {
    NULL_DATA = 0,
    ARRAY = 1,
    STRUCT = 2,
    BOOLEAN = 3,
    DOUBLE = 6,
    OCTET_STRING = 9,
    INTEGER = 15,
    LONG = 16,
    UNSIGNED = 17,
    ENUM = 22,
};

union dlms_payload {
    void* raw;
    struct dlms_object* OBJECT;
    uint8_t* HEX_STRING;
    bool* BOOLEAN;
    int32_t* DOUBLE;
    int8_t* INTEGER;
    uint8_t* UNSIGNED;
    int16_t* LONG;
};

struct dlms_object {
    enum dlms_data_type type;
    uint8_t size;
    union dlms_payload payload;
};

size_t dlms_parse_object(struct dlms_object* dest, char* buf);


void dlms_free_object(struct dlms_object);
