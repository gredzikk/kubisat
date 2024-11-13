// BME280_WRAPPER.h
#ifndef BME280_WRAPPER_H
#define BME280_WRAPPER_H

#include "ISensor.h"
#include "BME280.h"

class BME280Wrapper : public ISensor {
private:
    BME280 sensor;
    bool initialized = false;

public:
    BME280Wrapper(i2c_inst_t* i2c);

    bool init() override;
    float readData(DataType type) override;
    bool isInitialized() const override;
    SensorType getType() const override;
        bool configure(const std::map<std::string, std::string>& config) override;

};

#endif // BME280_WRAPPER_H