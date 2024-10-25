#include "data_types.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

struct cosem_data cosem_parse_data(struct dlms_object obj) {
    struct cosem_data result;
    result.logical_name = obj.payload.OBJECT[0].payload.OCTET_STRING;
    result.value = obj.payload.OBJECT[1];
    return result;
}

struct cosem_register cosem_parse_register(struct dlms_object obj) {
    struct cosem_register result;
    result.logical_name = obj.payload.OBJECT[0].payload.OCTET_STRING;
    result.value = obj.payload.OBJECT[1];
    result.scaler_unit = cosem_parse_scaler_unit(obj.payload.OBJECT[2]);
    return result;
}

struct cosem_scaler_unit cosem_parse_scaler_unit(struct dlms_object obj) {
    struct cosem_scaler_unit result;
    result.scaler = *obj.payload.OBJECT[0].payload.INTEGER;
    result.unit = *obj.payload.OBJECT[1].payload.UNSIGNED;
    return result;
}

double cosem_get_scaled_value(struct cosem_register reg) {
    double value = *reg.value.payload.LONG * pow(10, reg.scaler_unit.scaler);
    return value;
}

size_t data_type_len(enum dlms_data_type type) { 
    switch (type) {
        case BOOLEAN:
        case INTEGER:
        case UNSIGNED:
        case ENUM:
            return 1;
            break;

        case LONG:
            return 2;
            break;

        case DOUBLE:
            return 4;
            break;

        // Types with non fixed sizes (and NULL)
        default:
            return 0;
            break;
    }
}

void* reverse_bytes(void *inp, size_t len)
{
    unsigned int i;
    unsigned char *in=(unsigned char *)inp,tmp;

    for(i=0;i<len/2;i++) {
        tmp=*(in+i);
        *(in+i)=*(in+len-i-1);
        *(in+len-i-1)=tmp;
    }

    return inp;
}

size_t dlms_parse_payload(struct dlms_object* obj, char* ptr) {
    union dlms_payload result;
    size_t size = 0;
    result.raw = 0;
    switch(obj->type) {
        case STRUCT:
            result.raw = malloc(sizeof(struct dlms_object) * obj->size);
            for(int i = 0; i < obj->size; i++) {
                size += dlms_parse_object((struct dlms_object*)result.raw + i, ptr + size);
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
            result.raw = reverse_bytes(result.raw, size);
            break;


        case NULL_DATA:
            size = 0;
    }   

    obj->payload = result;
    return size;
}

/**
 * @brief Get the size of an object from pdu
 * 
 * This method returns type bytes and payload sizes separetely.
 * The type bytes is the bytes that secify the data type of the object.
 * If the object has a fixed length data type (such as INTEGER), there is 1 type byte.
 * Variable length types (such as ARRAY or BIT_STRING) need 2 type bytes.
 * 
 * For data types ARRAY and STRUCT, number of elements is returned instead of size in bytes.
 * 
 * @param len Output variable where the size of the object (Excluding the type bytes) is written.
 * @param type_bytes Output variable where the number of type bytes is written.
 * @param size The raw bytes to parse.
 */
void get_size_from_pdu(uint8_t* size, uint8_t* type_bytes, char* pdu) {
    enum dlms_data_type type = (enum dlms_data_type)pdu[0];
    size[0] = data_type_len(type);

    bool fixed_size = *size || !type;
    type_bytes[0] = 1;

    if(!fixed_size) {
        size[0] = pdu[1];
        type_bytes[0] = 2;
    }
}

size_t dlms_parse_object(struct dlms_object* dest, char* pdu) {
    uint8_t type_bytes = 0;

    dest->type = pdu[0];
    get_size_from_pdu(&dest->size, &type_bytes, pdu);
    size_t payload_size = dlms_parse_payload(dest, pdu + type_bytes);

    return payload_size + type_bytes;
}

void dlms_free_object(struct dlms_object obj) {
    if(obj.type == ARRAY) {
        return;
    } 
    else if(obj.type == STRUCT) {
        for(int i = 0; i < obj.size; i++) {
            dlms_free_object(obj.payload.OBJECT[i]);
        }
    }
    if(obj.size > 0) {
        free(obj.payload.raw);
    }
}