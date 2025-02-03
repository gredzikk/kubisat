#include "data_types.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "data_types.cpp"

int main() {
    dlms_object sub_objects[2];
    int8_t sub_integer_payload = -1;
    uint8_t sub_unsigned_payload = 255;

    sub_objects[0] = create_dlms_object(INTEGER, sizeof(sub_integer_payload), &sub_integer_payload);
    sub_objects[1] = create_dlms_object(UNSIGNED, sizeof(sub_unsigned_payload), &sub_unsigned_payload);

    dlms_object register_object_1 = create_dlms_object(STRUCT, 2, sub_objects);

    print_dlms_object(register_object_1);

    logical_name ln1;
    char* temp_ln1 = create_logical_name(INA3221, DATA_REGISTER_1);
    if (temp_ln1 != nullptr) {
        std::strncpy(reinterpret_cast<char*>(ln1), temp_ln1, sizeof(logical_name));
        std::free(temp_ln1);
    }
    else {
        std::cerr << "Memory allocation failed for logical name" << std::endl;
        return 1;
    }

    cosem_data data = create_cosem_data(ln1, register_object_1);
    std::cout << "\nType: " << register_object_1.type << std::endl;
    print_cosem_data(data);

    dlms_free_object(register_object_1);

    int8_t value = 2137;
    dlms_object integer_data = create_dlms_object(INTEGER, sizeof(value), &value);

    logical_name ln2;
    char* temp_ln2 = create_logical_name(MPU6050, DATA_REGISTER_2);
    if (temp_ln2 != nullptr) {
        std::strncpy(reinterpret_cast<char*>(ln2), temp_ln2, sizeof(logical_name));
        std::free(temp_ln2);
    }
    else {
        std::cerr << "Memory allocation failed for logical name" << std::endl;
        return 1;
    }

    cosem_data data1 = create_cosem_data(ln2, integer_data);
    print_cosem_data(data1);

    dlms_free_object(integer_data);

    return 0;
}
