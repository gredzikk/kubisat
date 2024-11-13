#ifndef HMC5883L_HPP
#define HMC5883L_HPP

#include "data_types.hpp"

class HMC5883L : public Sensor {
public:
    HMC5883L();
    SensorData* read_register_data(DataRegister register_);
};

#endif // HMC5883L_HPP
