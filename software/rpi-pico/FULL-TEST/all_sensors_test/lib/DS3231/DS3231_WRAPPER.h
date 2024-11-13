// DS3231_WRAPPER.h
#ifndef DS3231_WRAPPER_H
#define DS3231_WRAPPER_H

#include "ISensor.h"
#include "DS3231.h"

class DS3231Wrapper : public ISensor {
private:
    DS3231 sensor;
    bool initialized = false;
    datetime_t current_time;

public:
    DS3231Wrapper(i2c_inst_t* i2c);

    bool init() override;
    float readData(DataType type) override;
    bool isInitialized() const override;
    SensorType getType() const override;

    // Additional helper method to set time if needed
    bool setTime(datetime_t* dt);
    bool configure(const std::map<std::string, std::string>& config) override;

};

#endif // DS3231_WRAPPER_H