#ifndef BMP280_HPP
#define BMP280_HPP

#include "data_types.hpp"

class BMP280 : public Sensor {
public:
    BMP280();
    SensorData* read_register_data(DataRegister register_) override; 
};

#endif // BMP280_HPP
