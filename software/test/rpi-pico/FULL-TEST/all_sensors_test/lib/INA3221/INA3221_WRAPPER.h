// INA3221_WRAPPER.h
#ifndef INA3221_WRAPPER_H
#define INA3221_WRAPPER_H

#include "ISensor.h"
#include "INA3221.h"

class INA3221Wrapper : public ISensor {
private:
    INA3221 sensor;
    bool initialized = false;

public:
    INA3221Wrapper(i2c_inst_t* i2c);

    bool init() override;
    float readData(DataType type) override;
    bool isInitialized() const override;
    SensorType getType() const override;
    bool configure(const std::map<std::string, std::string>& config) override;

};

#endif // INA3221_WRAPPER_H