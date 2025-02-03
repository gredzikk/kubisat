#include "data_types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    logical_name ln1;
    char* temp_ln = create_logical_name(INA3221, DATA_REGISTER_1);
    if (temp_ln != NULL) {
        strncpy((char*)ln1, temp_ln, sizeof(logical_name));
        free(temp_ln);
    }
    else {
        fprintf(stderr, "Memory allocation failed for logical name\n");
        return 1;
    }

    struct dlms_object sub_objects[2];
    int8_t sub_integer_payload = -1;
    uint8_t sub_unsigned_payload = 255;

    sub_objects[0] = create_dlms_object(INTEGER, sizeof(sub_integer_payload), &sub_integer_payload);
    if (sub_objects[0].payload.INTEGER == NULL) {
        fprintf(stderr, "Memory allocation failed for sub_objects[0].payload.INTEGER\n");
        return 1;
    }

    sub_objects[1] = create_dlms_object(UNSIGNED, sizeof(sub_unsigned_payload), &sub_unsigned_payload);
    if (sub_objects[1].payload.UNSIGNED == NULL) {
        fprintf(stderr, "Memory allocation failed for sub_objects[1].payload.UNSIGNED\n");
        dlms_free_object(sub_objects[0]);
        return 1;
    }

    for (int i = 0; i < 2; i++) {
        print_dlms_object(sub_objects[i]);
    }

    struct dlms_object register_object_1 = create_dlms_object(STRUCT, 2, sub_objects);
    if (register_object_1.payload.OBJECT == NULL) {
        fprintf(stderr, "Memory allocation failed for obj3.payload.OBJECT\n");
        dlms_free_object(sub_objects[0]);
        dlms_free_object(sub_objects[1]);
        return 1;
    }

    struct cosem_data data = create_cosem_data(ln1, register_object_1);
    print_cosem_data(data);

    dlms_free_object(register_object_1);

    return 0;
}
#pragma once
