// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef BME280_WRAPPER_H
#define BME280_WRAPPER_H

#include "ISensor.h"
#include "BME280.h"

class BME280Wrapper : public ISensor
{
private:
    BME280 sensor_;
    bool initialized_ = false;

public:
    BME280Wrapper(i2c_inst_t *i2c);

    bool init() override;
    float read_data(SensorDataTypeIdentifier type) override;
    bool is_initialized() const override;
    SensorType get_type() const override;
    bool configure(const std::map<std::string, std::string> &config) override;

    uint8_t get_address() const override
    {
        return 0x76;
    }
};

#endif // BME280_WRAPPER_H