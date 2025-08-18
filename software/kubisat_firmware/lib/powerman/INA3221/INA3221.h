// Copyright (c) 2025 Jakub Grędzicki
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#ifndef BEASTDEVICES_INA3221_H
#define BEASTDEVICES_INA3221_H

#include <stdint.h>
#include <iostream>
#include <hardware/i2c.h>

/**
 * @file INA3221.h
 * @brief Header file for the INA3221 triple-channel power monitor driver
 */
typedef enum
{
    INA3221_ADDR40_GND = 0b1000000, // A0 pin -> GND
    INA3221_ADDR41_VCC = 0b1000001, // A0 pin -> VCC
    INA3221_ADDR42_SDA = 0b1000010, // A0 pin -> SDA
    INA3221_ADDR43_SCL = 0b1000011  // A0 pin -> SCL
} ina3221_addr_t;

typedef enum
{
    INA3221_CH1 = 0,
    INA3221_CH2,
    INA3221_CH3,
} ina3221_ch_t;

/** @brief Number of channels in INA3221 */
const int INA3221_CH_NUM = 3;
/** @brief LSB value for shunt voltage measurements in microvolts */
const int SHUNT_VOLTAGE_LSB_UV = 5;

/**
 * @brief Register addresses for INA3221
 */
typedef enum
{
    INA3221_REG_CONF = 0,
    INA3221_REG_CH1_SHUNTV,
    INA3221_REG_CH1_BUSV,
    INA3221_REG_CH2_SHUNTV,
    INA3221_REG_CH2_BUSV,
    INA3221_REG_CH3_SHUNTV,
    INA3221_REG_CH3_BUSV,
    INA3221_REG_CH1_CRIT_ALERT_LIM,
    INA3221_REG_CH1_WARNING_ALERT_LIM,
    INA3221_REG_CH2_CRIT_ALERT_LIM,
    INA3221_REG_CH2_WARNING_ALERT_LIM,
    INA3221_REG_CH3_CRIT_ALERT_LIM,
    INA3221_REG_CH3_WARNING_ALERT_LIM,
    INA3221_REG_SHUNTV_SUM,
    INA3221_REG_SHUNTV_SUM_LIM,
    INA3221_REG_MASK_ENABLE,
    INA3221_REG_PWR_VALID_HI_LIM,
    INA3221_REG_PWR_VALID_LO_LIM,
    INA3221_REG_MANUF_ID = 0xFE,
    INA3221_REG_DIE_ID = 0xFF
} ina3221_reg_t;

/**
 * @brief Averaging mode settings
 * @details Number of samples to average for each measurement
 */
typedef enum
{
    INA3221_REG_CONF_AVG_1 = 0,
    INA3221_REG_CONF_AVG_4,
    INA3221_REG_CONF_AVG_16,
    INA3221_REG_CONF_AVG_64,
    INA3221_REG_CONF_AVG_128,
    INA3221_REG_CONF_AVG_256,
    INA3221_REG_CONF_AVG_512,
    INA3221_REG_CONF_AVG_1024
} ina3221_avg_mode_t;

/**
 * @brief Operating mode settings
 * @details Continuous or triggered (single-shot) mode
 */
typedef enum
{
    CONTINUOUS_MODE,
    TRIGGERED_MODE
} ina3221_op_mode_t;

/**
 * @brief INA3221 Triple-Channel Power Monitor driver class
 * @details Provides functionality for voltage, current, and power monitoring
 *          with configurable alerts and power valid monitoring
 */
class INA3221
{

    /**
     * @brief Configuration register bit fields
     */
    typedef struct
    {
        uint16_t mode_shunt_en : 1;
        uint16_t mode_bus_en : 1;
        uint16_t mode_continious_en : 1;
        uint16_t shunt_conv_time : 3;
        uint16_t bus_conv_time : 3;
        uint16_t avg_mode : 3;
        uint16_t ch3_en : 1;
        uint16_t ch2_en : 1;
        uint16_t ch1_en : 1;
        uint16_t reset : 1;
    } conf_reg_t;

    /**
     * @brief Mask/Enable register bit fields
     */
    typedef struct
    {
        uint16_t conv_ready : 1;
        uint16_t timing_ctrl_alert : 1;
        uint16_t pwr_valid_alert : 1;
        uint16_t warn_alert_ch3 : 1;
        uint16_t warn_alert_ch2 : 1;
        uint16_t warn_alert_ch1 : 1;
        uint16_t shunt_sum_alert : 1;
        uint16_t crit_alert_ch3 : 1;
        uint16_t crit_alert_ch2 : 1;
        uint16_t crit_alert_ch1 : 1;
        uint16_t crit_alert_latch_en : 1;
        uint16_t warn_alert_latch_en : 1;
        uint16_t shunt_sum_en_ch3 : 1;
        uint16_t shunt_sum_en_ch2 : 1;
        uint16_t shunt_sum_en_ch1 : 1;
        uint16_t reserved : 1;
    } masken_reg_t;

    // I2C address
    ina3221_addr_t _i2c_addr;
    i2c_inst_t *_i2c;

    // Shunt resistance in mOhm
    uint32_t _shuntRes[INA3221_CH_NUM];

    // Series filter resistance in Ohm
    uint32_t _filterRes[INA3221_CH_NUM];

    // Value of Mask/Enable register.
    masken_reg_t _masken_reg;

    // Reads 16 bytes from a register.
    void _read(ina3221_reg_t reg, uint16_t *val);

    // Writes 16 bytes to a register.
    void _write(ina3221_reg_t reg, uint16_t *val);

public:
    INA3221(ina3221_addr_t addr, i2c_inst_t *i2c);
    // Initializes INA3221
    bool begin();

    // Gets a register value.
    uint16_t read_register(ina3221_reg_t reg);

    // Sets operating mode to continious
    void set_mode_continuous();

    // Sets operating mode to triggered (single-shot)
    void set_mode_triggered();

    // Sets averaging mode. Sets number of samples that are collected
    // and averaged togehter.
    void set_averaging_mode(ina3221_avg_mode_t mode);

    // Gets manufacturer ID.
    // Should read 0x5449.
    uint16_t get_manufacturer_id();

    // Gets die ID.
    // Should read 0x3220.
    uint16_t get_die_id();

    // Gets shunt voltage in uV.
    int32_t get_shunt_voltage(ina3221_ch_t channel);

    // Gets current in mA.
    float get_current_ma(ina3221_ch_t channel);

    // Gets bus voltage in V.
    float get_voltage(ina3221_ch_t channel);
};

#endif