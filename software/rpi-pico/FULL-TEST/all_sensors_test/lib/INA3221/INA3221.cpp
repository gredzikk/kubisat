#include "INA3221.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <iostream>

void INA3221::begin() {
    _shuntRes[0] = 10;
    _shuntRes[1] = 10;
    _shuntRes[2] = 10;

    _filterRes[0] = 10;
    _filterRes[1] = 10;
    _filterRes[2] = 10;
}

void INA3221::_read(ina3221_reg_t reg, uint16_t *val) {
    uint8_t reg_buf = reg;
    uint8_t data[2];

    int ret = i2c_write_blocking(i2c_default, _i2c_addr, &reg_buf, 1, true);
    if (ret != 1) {
        std::cerr << "Failed to write register address to I2C device." << std::endl;
        return;
    }

    ret = i2c_read_blocking(i2c_default, _i2c_addr, data, 2, false);
    if (ret != 2) {
        std::cerr << "Failed to read data from I2C device." << std::endl;
        return;
    }

    *val = (data[0] << 8) | data[1];
}

void INA3221::_write(ina3221_reg_t reg, uint16_t *val) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (*val >> 8) & 0xFF; // MSB
    buf[2] = (*val) & 0xFF;      // LSB

    int ret = i2c_write_blocking(i2c_default, _i2c_addr, buf, 3, false);
    if (ret != 3) {
        std::cerr << "Failed to write data to I2C device." << std::endl;
    }
}

uint16_t INA3221::getReg(ina3221_reg_t reg){
    uint16_t val = 0;
    _read(reg, &val);
    return val;
}

void INA3221::reset(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.reset = 1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setModePowerDown(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_bus_en = 0;
    conf_reg.mode_continious_en =0 ;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setModeContinious(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_continious_en =1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setModeTriggered(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_continious_en = 0;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setShuntMeasEnable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_shunt_en = 1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setShuntMeasDisable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_shunt_en = 0;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setBusMeasEnable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_bus_en = 1;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setBusMeasDisable(){
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.mode_bus_en = 0;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setAveragingMode(ina3221_avg_mode_t mode) {
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.avg_mode = mode;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setBusConversionTime(ina3221_conv_time_t convTime) {
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.bus_conv_time = convTime;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

void INA3221::setShuntConversionTime(ina3221_conv_time_t convTime) {
    conf_reg_t conf_reg;

    _read(INA3221_REG_CONF, (uint16_t*)&conf_reg);
    conf_reg.shunt_conv_time = convTime;
    _write(INA3221_REG_CONF, (uint16_t*)&conf_reg);
}

uint16_t INA3221::getManufID() {
    uint16_t id = 0;
    _read(INA3221_REG_MANUF_ID, &id);
    return id;
}

uint16_t INA3221::getDieID() {
    uint16_t id = 0;
    _read(INA3221_REG_DIE_ID, &id);
    return id;
}

int32_t INA3221::getShuntVoltage(ina3221_ch_t channel) {
    int32_t res;
    ina3221_reg_t reg;
    uint16_t val_raw = 0;

    switch(channel){
        case INA3221_CH1:
            reg = INA3221_REG_CH1_SHUNTV;
            break;
        case INA3221_CH2:
            reg = INA3221_REG_CH2_SHUNTV;
            break;
        case INA3221_CH3:
            reg = INA3221_REG_CH3_SHUNTV;
            break;
    }

    _read(reg, &val_raw);

    res = (int16_t) (val_raw >> 3);
    res *= SHUNT_VOLTAGE_LSB_UV; 

    return res;
}

int32_t INA3221::estimateOffsetVoltage(ina3221_ch_t channel, uint32_t busV) {
    float bias_in = 10.0;        // Input bias current at IN– in uA
    float r_in = 0.670;         // Input resistance at IN– in MOhm
    uint32_t adc_step = 40;      // smallest shunt ADC step in uV
    float shunt_res = _shuntRes[channel]/1000.0; // convert to Ohm
    float filter_res = _filterRes[channel];
    int32_t offset = 0.0;
    float reminder;

    offset = (shunt_res + filter_res)*(busV/r_in + bias_in) - bias_in * filter_res;

    // Round the offset to the closest shunt ADC value
    reminder = offset % adc_step;
    if (reminder < adc_step/2)
    {
        offset -= reminder;
    } else {
        offset += adc_step - reminder;
    }

    return offset;
}

float INA3221::getCurrent(ina3221_ch_t channel) {
    int32_t shunt_uV = 0;
    float current_A = 0;

    shunt_uV = getShuntVoltage(channel);
    current_A = shunt_uV / (int32_t)_shuntRes[channel] / 1000.0;
    return current_A;
}

float INA3221::getCurrent_mA(ina3221_ch_t channel) {
    int32_t shunt_uV = 0;
    float current_A = 0;

    shunt_uV = getShuntVoltage(channel);
    current_A = shunt_uV / (int32_t)_shuntRes[channel];
    return current_A;
}

float INA3221::getCurrentCompensated(ina3221_ch_t channel) {
    int32_t shunt_uV = 0;
    int32_t bus_V = 0;
    float current_A = 0.0;
    int32_t offset_uV = 0;

    shunt_uV = getShuntVoltage(channel);
    bus_V = getVoltage(channel);
    offset_uV = estimateOffsetVoltage(channel, bus_V);

    current_A = (shunt_uV - offset_uV) / (int32_t)_shuntRes[channel] / 1000.0;

    return current_A;
}

float INA3221::getCurrentCompensated_mA(ina3221_ch_t channel) {
    int32_t shunt_uV = 0;
    int32_t bus_V = 0;
    float current_A = 0.0;
    int32_t offset_uV = 0;

    shunt_uV = getShuntVoltage(channel);
    bus_V = getVoltage(channel);
    offset_uV = estimateOffsetVoltage(channel, bus_V);

    current_A = (shunt_uV - offset_uV) / (int32_t)_shuntRes[channel];

    return current_A;
}

float INA3221::getVoltage(ina3221_ch_t channel) {
    float voltage_V = 0.0;
    ina3221_reg_t reg;
    uint16_t val_raw = 0;

    switch(channel){
        case INA3221_CH1:
            reg = INA3221_REG_CH1_BUSV;
            break;
        case INA3221_CH2:
            reg = INA3221_REG_CH2_BUSV;
            break;
        case INA3221_CH3:
            reg = INA3221_REG_CH3_BUSV;
            break;
    }

    _read(reg, &val_raw);
    voltage_V = val_raw / 1000.0;
    return voltage_V;
}
