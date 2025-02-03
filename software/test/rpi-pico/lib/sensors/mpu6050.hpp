#ifndef MPU6050_HPP
#define MPU6050_HPP

#include "data_types.hpp"

class MPU6050 : public Sensor {
public:
    MPU6050();
    SensorData* read_register_data(DataRegister register_);
};

#endif // MPU6050_HPP

