#include "data_types.h"
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <memory>

std::unique_ptr<uint8_t[]> create_logical_name(sensor_type sensor, DataRegister data) {
    auto logical_name = std::make_unique<uint8_t[]>(5); // "XXYY\0"
    snprintf(reinterpret_cast<char*>(logical_name.get()), 5, "%02d%02d", sensor, data);
    return logical_name;
}

void parse_logical_name(const char* logical_name, int* sensor_id, int* data_id) {
    sscanf(logical_name, "%d.%d", sensor_id, data_id);
}

cosem_data cosem_parse_data(const dlms_object& obj) {
    cosem_data result;
    result.logical_name = obj.payload.OBJECT[0].payload.HEX_STRING;
    result.value = obj.payload.OBJECT[1];
    return result;
}

cosem_register cosem_parse_register(const dlms_object& obj) {
    cosem_register result;
    result.logical_name = obj.payload.OBJECT[0].payload.HEX_STRING;
    result.value = obj.payload.OBJECT[1];
    result.scaler_unit = cosem_parse_scaler_unit(obj.payload.OBJECT[2]);
    return result;
}

cosem_scaler_unit cosem_parse_scaler_unit(const dlms_object& obj) {
    cosem_scaler_unit result;
    result.scaler = *obj.payload.OBJECT[0].payload.INTEGER;
    result.unit = *obj.payload.OBJECT[1].payload.UNSIGNED;
    return result;
}

double cosem_get_scaled_value(const cosem_register& reg) {
    return *reg.value.payload.LONG * std::pow(10, reg.scaler_unit.scaler);
}

void print_dlms_object(const dlms_object& obj) {
    switch (obj.type) {
    case BOOLEAN:
        printf("Type:  BOOLEAN\n");
        printf("Value: %s\n", *obj.payload.BOOL ? "true" : "false");
        break;
    case INTEGER:
        printf("Type:  INTEGER\n");
        printf("Value: %d\n", *obj.payload.INTEGER);
        break;
    case UNSIGNED:
        printf("Type:  UNSIGNED\n");
        printf("Value: %u\n", *obj.payload.UNSIGNED);
        break;
    case LONG:
        printf("Type:  LONG\n");
        printf("Value: %d\n", *obj.payload.LONG);
        break;
    case DOUBLE:
        printf("Type:  DOUBLE\n");
        printf("Value: %f\n", *obj.payload.DOUBLE);
        break;
    case HEX_STRING:
        printf("Type:  HEX_STRING\n");
        printf("Value: %s\n", obj.payload.HEX_STRING);
        break;
    case STRUCT:
        printf("Type:  STRUCT\n");
        for (size_t i = 0; i < obj.size; i++) {
            printf("  Element %zu:\n", i);
            print_dlms_object(obj.payload.OBJECT[i]);
        }
        break;
    case ARRAY:
        printf("Type:  ARRAY\n");
        for (size_t i = 0; i < obj.size; i++) {
            printf("  Element %zu:\n", i);
            print_dlms_object(obj.payload.OBJECT[i]);
        }
        break;
    default:
        printf("Type:  Unknown\n");
        break;
    }
}

void print_cosem_data(const cosem_data& data) {
    printf("COSEM Data:\n");
    printf("  Logical Name: %s\n", data.logical_name);
    print_dlms_object(data.value);
}

void print_cosem_register(const cosem_register& reg) {
    printf("COSEM Register:\n");
    printf("  Logical Name: %s\n", reg.logical_name);
    printf("  Value: %d\n", *reg.value.payload.INTEGER);
    printf("    Scaler: %d\n", reg.scaler_unit.scaler);
    printf("    Unit: %u\n", reg.scaler_unit.unit);
}

void print_scaled_value(const cosem_register& reg) {
    double scaled_value = cosem_get_scaled_value(reg);
    printf("Scaled Value: %f\n", scaled_value);
}

size_t data_type_len(dlms_data_type type) {
    switch (type) {
    case BOOLEAN:
    case INTEGER:
    case UNSIGNED:
    case ENUM:
        return 1;
    case LONG:
        return 2;
    case DOUBLE:
        return 4;
    default:
        return 0;
    }
}

void* reverse_bytes(void* inp, size_t len) {
    unsigned char* in = static_cast<unsigned char*>(inp);
    for (size_t i = 0; i < len / 2; i++) {
        std::swap(in[i], in[len - i - 1]);
    }
    return inp;
}

size_t dlms_parse_payload(dlms_object* obj, char* ptr) {
    union dlms_payload result;
    size_t size = 0;
    result.raw = nullptr;
    switch (obj->type) {
    case STRUCT:
        result.raw = malloc(sizeof(dlms_object) * obj->size);
        for (int i = 0; i < obj->size; i++) {
            size += dlms_parse_object(reinterpret_cast<dlms_object*>(result.raw) + i, ptr + size);
        }
        break;
    case ARRAY:
        result.raw = ptr;
        break;
    case HEX_STRING:
        size = obj->size;
        result.raw = malloc(size);
        memcpy(result.raw, ptr, size);
        break;
    case BOOLEAN:
    case DOUBLE:
    case INTEGER:
    case LONG:
    case UNSIGNED:
    case ENUM:
        size = obj->size;
        result.raw = malloc(size);
        memcpy(result.raw, ptr, size);
        reverse_bytes(result.raw, size);
        break;
    case NULL_DATA:
        size = 0;
    }
    obj->payload = result;
    return size;
}

void get_size_from_pdu(uint8_t* size, uint8_t* type_bytes, char* pdu) {
    dlms_data_type type = static_cast<dlms_data_type>(pdu[0]);
    size[0] = data_type_len(type);

    bool fixed_size = *size || !type;
    type_bytes[0] = 1;

    if (!fixed_size) {
        size[0] = pdu[1];
        type_bytes[0] = 2;
    }
}

size_t dlms_parse_object(dlms_object* dest, char* pdu) {
    uint8_t type_bytes_local = 0;

    dest->type = static_cast<dlms_data_type>(pdu[0]);
    get_size_from_pdu(&dest->size, &type_bytes_local, pdu);
    size_t payload_size = dlms_parse_payload(dest, pdu + type_bytes_local);

    return payload_size + type_bytes_local;
}

void dlms_free_object(dlms_object& obj) {
    if (obj.type == ARRAY) {
        return;
    }
    else if (obj.type == STRUCT) {
        for (int i = 0; i < obj.size; i++) {
            dlms_free_object(obj.payload.OBJECT[i]);
        }
    }
    if (obj.size > 0) {
        free(obj.payload.raw);
    }
}

dlms_object create_dlms_object(dlms_data_type type, size_t size, void* payload) {
    dlms_object obj;
    obj.type = type;
    obj.size = size;
    switch (type) {
    case HEX_STRING:
        obj.payload.HEX_STRING = static_cast<uint8_t*>(malloc(size));
        if (obj.payload.HEX_STRING != nullptr) {
            memcpy(obj.payload.HEX_STRING, payload, size);
        }
        break;
    case INTEGER:
        obj.payload.INTEGER = static_cast<int8_t*>(malloc(size));
        if (obj.payload.INTEGER != nullptr) {
            memcpy(obj.payload.INTEGER, payload, size);
        }
        break;
    case UNSIGNED:
        obj.payload.UNSIGNED = static_cast<uint8_t*>(malloc(size));
        if (obj.payload.UNSIGNED != nullptr) {
            memcpy(obj.payload.UNSIGNED, payload, size);
        }
        break;
    case LONG:
        obj.payload.LONG = static_cast<int16_t*>(malloc(size));
        if (obj.payload.LONG != nullptr) {
            memcpy(obj.payload.LONG, payload, size);
        }
        break;
    case STRUCT:
        obj.payload.OBJECT = static_cast<dlms_object*>(malloc(size * sizeof(dlms_object)));
        if (obj.payload.OBJECT != nullptr) {
            memcpy(obj.payload.OBJECT, payload, size * sizeof(dlms_object));
        }
        break;
    default:
        obj.payload.raw = nullptr;
        break;
    }
    return obj;
}

cosem_data create_cosem_data(uint8_t* logical_name, dlms_object value) {
    cosem_data data_obj;
    data_obj.logical_name = logical_name;
    data_obj.value = value;
    return data_obj;
}

cosem_register create_cosem_register(uint8_t* logical_name, dlms_object value, cosem_scaler_unit scaler_unit) {
    cosem_register reg;
    reg.logical_name = logical_name;
    reg.value = value;
    reg.scaler_unit = scaler_unit;
    return reg;
}
