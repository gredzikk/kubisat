#include "bmp280.hpp"
#include "hmc5883l.hpp"
#include "ina3221.hpp"
#include "mpu6050.hpp"
#include "neo6m.hpp"
#include "data_types.hpp"
#include <iostream>
#include <vector>

// Funkcja zwracaj¹ca listê wszystkich dostêpnych czujników
std::vector<Sensor*> get_all_sensors() {
    // Dodaj wszystkie czujniki
    return { new BMP280(), new INA3221(), new NEO_6M(), new MPU6050(), new HMC5883L() };
}

int main() {
    // Pobierz listê wszystkich czujników
    std::vector<Sensor*> sensors = get_all_sensors();

    // Lista rejestrów do odczytu
    std::vector<DataRegister> registers = {
        DataRegister::DATA_REGISTER_0,
        DataRegister::DATA_REGISTER_1,
        DataRegister::DATA_REGISTER_2,
        DataRegister::DATA_REGISTER_3,
        DataRegister::DATA_REGISTER_4,
        DataRegister::DATA_REGISTER_5,
        DataRegister::DATA_REGISTER_6,
        DataRegister::DATA_REGISTER_7,
        DataRegister::DATA_REGISTER_8,
    };

    for (Sensor* sensor : sensors) {
        for (const auto& reg : registers) {
            try {
                std::cout << "\n========================================================================" << std::endl;
                SensorData* data = sensor->read_register_data(reg);
				std::cout << "Sensor type: " << static_cast<int>(data->sensor_type) << ", register " << static_cast<int>(data->sensor_type) << std::endl;
				std::string serialized = serialize_sensor_data(data);
				SensorData* deserialized = deserialize_sensor_data(serialized);
                pretty_print_sensor_data(data);
                delete data; 
            }
            catch (const std::invalid_argument& e) {
                std::cerr << "Error reading register " << static_cast<int>(reg)
                    << " from sensor " << static_cast<int>(sensor->sensor_type) << ": " << e.what() << "\n";
            }
        }
        delete sensor; // Pamiêtaj o zwolnieniu pamiêci
    }

    return 0;
}
