#ifndef NEO6M_HPP
#define NEO6M_HPP

#include "data_types.hpp"

class NEO_6M : public Sensor {
public:
    NEO_6M();
    SensorData* read_register_data(DataRegister register_);
};

#endif // NEO6M_HPP

