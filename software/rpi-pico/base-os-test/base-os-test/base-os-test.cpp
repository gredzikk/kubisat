#include <stdio.h>
#include "pico/stdlib.h"
#include <pico/binary_info/code.h>
#include <vector>
#include "sensors/data_types.hpp"
#include "sensors/bmp280.hpp"

int main() {
    stdio_init_all();

    BMP280 bmp;

    while (true) {
        SensorData* data = bmp.read_register_data(DataRegister::DATA_REGISTER_2);
        if (data) {
            std::string serialized = serialize_sensor_data(data);
            SensorData* deserialized = deserialize_sensor_data(serialized);
            pretty_print_sensor_data(deserialized);
            delete deserialized;
            delete data;
        }
        sleep_ms(1000);
    }

    return 0;
}


