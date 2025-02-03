#ifndef INA3221_HPP
#define INA3221_HPP

#include "data_types.hpp"

class INA3221 : public Sensor {
public:
    INA3221();
    SensorData* read_register_data(DataRegister register_);
};

#endif // INA3221_HPP
