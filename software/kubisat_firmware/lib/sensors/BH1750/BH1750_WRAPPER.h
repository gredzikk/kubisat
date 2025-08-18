// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef BH1750_WRAPPER_H
#define BH1750_WRAPPER_H

#include "ISensor.h"
#include "BH1750.h"
#include <map>
#include <string>

class BH1750Wrapper : public ISensor
{
private:
    BH1750 sensor_;
    bool initialized_ = false;

public:
    BH1750Wrapper(i2c_inst_t *i2c);
    BH1750Wrapper();
    int get_i2c_addr();
    bool init() override;
    float read_data(SensorDataTypeIdentifier type) override;
    bool is_initialized() const override;
    SensorType get_type() const override;

    bool configure(const std::map<std::string, std::string> &config);

    uint8_t get_address() const override
    {
        return 0x23;
    }
};

#endif // BH1750_WRAPPER_H