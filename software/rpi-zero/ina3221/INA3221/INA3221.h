#ifndef INA3221_H
#define INA3221_H

#include <wiringPiI2C.h>
#include <stdint.h>
#include <list>

class INA3221 {
public:
    #define ResistanceShunt 0.01
    #define RESET 0b1000000000000000
    #define TI 0x5449
    #define INA3221 0x3220
    #define _40MICROVOLT 0.04
    #define _8MILLIVOLT 0.008

    enum Address {
        ADDR_40 = 0x40,
        ADDR_41 = 0x41,
        ADDR_42 = 0x42,
        ADDR_43 = 0x43
    };

    enum Channel {
        Channel_1 = 1,
        Channel_2 = 2,
        Channel_3 = 3
    };

    enum Register {
        Config = 0x00,
        Channel1ShuntVoltage = 0x01,
        Channel1BusVoltage = 0x02,
        Channel2ShuntVoltage = 0x03,
        Channel2BusVoltage = 0x04,
        Channel3ShuntVoltage = 0x05,
        Channel3BusVoltage = 0x06,
        Channel1CriticalAlertLimit = 0x07,
        Channel1WarningAlertLimit = 0x08,
        Channel2CriticalAlertLimit = 0x09,
        Channel2WarningAlertLimit = 0x0A,
        Channel3CriticalAlertLimit = 0x0B,
        Channel3WarningAlertLimit = 0x0C,
        ShuntVoltageSum = 0x0D,
        ShuntVoltageSumLimit = 0x0E,
        MaskEnable = 0x0F,
        PowerValidUpperLimit = 0x10,
        PowerValidLowerLimit = 0x11,
        ManufacturerID = 0xFE,
        DieID = 0xFF
    };

    enum Channel1 {
        Channel1On  = 0b0100000000000000,
        Channel1Off = 0b0000000000000000
    };
        
    enum Channel2 {
        Channel2On  = 0b0010000000000000,
        Channel2Off = 0b0000000000000000
    };

    enum Channel3 {
        Channel3On  = 0b0001000000000000,
        Channel3Off = 0b0000000000000000
    };

    enum Average {
        Avg_1    = 0b0000000000000000,
        Avg_4    = 0b0000001000000000,
        Avg_16   = 0b0000010000000000,
        Avg_64   = 0b0000011000000000,
        Avg_128  = 0b0000100000000000,
        Avg_256  = 0b0000101000000000,
        Avg_512  = 0b0000110000000000,
        Avg_1024 = 0b0000111000000000
    };

    enum BusVoltageConversionTime {	
        VBUS_140US  = 0b0000000000000000,
        VBUS_204US  = 0b0000000001000000,
        VBUS_332US  = 0b0000000010000000,
        VBUS_588US  = 0b0000000011000000,
        VBUS_1100US = 0b0000000100000000,
        VBUS_2116US = 0b0000000101000000,
        VBUS_4156US = 0b0000000110000000,
        VBUS_8244US = 0b0000000111000000
    };

    enum ShuntVoltageConversionTime {	
        SBUS_140US  = 0b0000000000000000,
        SBUS_204US  = 0b0000000000001000,
        SBUS_332US  = 0b0000000000010000,
        SBUS_588US  = 0b0000000000011000,
        SBUS_1100US = 0b0000000000100000,
        SBUS_2116US = 0b0000000000101000,
        SBUS_4156US = 0b0000000000110000,
        SBUS_8244US = 0b0000000000111000
    };

    enum Mode {
        PowerDown                    = 0b0000000000000000,
        ShuntVoltageTriggered        = 0b0000000000000001,
        BusVoltageTrigerred          = 0b0000000000000010,
        ShuntAndBusVoltageTriggered  = 0b0000000000000011,
        ShuntVoltageContinuous       = 0b0000000000000101,
        BusVoltageContinuous         = 0b0000000000000110,
        ShuntAndBusVoltageContinuous = 0b0000000000000111
    };

    enum SummationChannelControl1 {
        SummationChannelControl1On  = 0b0100000000000000,
        SummationChannelControl1Off = 0b0000000000000000
    };

    enum SummationChannelControl2 {
        SummationChannelControl2On  = 0b0010000000000000,
        SummationChannelControl2Off = 0b0000000000000000
    };

    enum SummationChannelControl3 {
        SummationChannelControl3On  = 0b0001000000000000,
        SummationChannelControl3Off = 0b0000000000000000
    };

    enum WarningAlertLatchEnable {
        WarningAlertLatchEnableOn  = 0b0000100000000000,
        WarningAlertLatchEnableOff = 0b0000000000000000
    };

    enum CriticalAlertLatchEnable {
        CriticalAlertLatchEnableOn  = 0b0000010000000000,
        CriticalAlertLatchEnableOff = 0b0000000000000000
    };

    enum CriticalAlertFlagIndicator1 {
        CriticalAlertFlagIndicator1On  = 0b0000001000000000,
        CriticalAlertFlagIndicator1Off = 0b0000000000000000
    };

    enum CriticalAlertFlagIndicator2 {
        CriticalAlertFlagIndicator2On  = 0b0000000100000000,
        CriticalAlertFlagIndicator2Off = 0b0000000000000000
    };

    enum CriticalAlertFlagIndicator3 {
        CriticalAlertFlagIndicator3On  = 0b0000000010000000,
        CriticalAlertFlagIndicator3Off = 0b0000000000000000
    };

    enum SummationAlertFlag {
        SummationAlertFlagOn  = 0b0000000001000000,
        SummationAlertFlagOff = 0b0000000000000000
    };

    enum WarningAlertFlagIndicator1 {
        WarningAlertFlagIndicator1On  = 0b0000000000100000,
        WarningAlertFlagIndicator1Off = 0b0000000000000000
    };

    enum WarningAlertFlagIndicator2 {
        WarningAlertFlagIndicator2On  = 0b0000000000010000,
        WarningAlertFlagIndicator2Off = 0b0000000000000000
    };

    enum WarningAlertFlagIndicator3 {
        WarningAlertFlagIndicator3On  = 0b0000000000001000,
        WarningAlertFlagIndicator3Off = 0b0000000000000000
    };

    enum PowerValidAlertFlag {
        PowerValidAlertFlagOn  = 0b0000000000000100,
        PowerValidAlertFlagOff = 0b0000000000000000
    };

    enum TimingControlAlertFlag {
        TimingControlAlertFlagOn  = 0b0000000000000010,
        TimingControlAlertFlagOff = 0b0000000000000000
    };

    enum ConversionReadyFlag {
        ConversionWriting = 0b0000000000000001,
        ConversionReady   = 0b0000000000000000
    };

    INA3221();
    INA3221(Address address);

    bool init(Address address);
    bool setConfiguration(Channel channel1, Channel channel2, Channel channel3, Average average, 
                          BusVoltageConversionTime busVoltageConversionTime, ShuntVoltageConversionTime shuntVoltageConversionTime, Mode mode);
    bool isChannelActive(Channel channel);
    Average getAverage();
    BusVoltageConversionTime getBusVoltageConversionTime();
    ShuntVoltageConversionTime getShuntVoltageConversionTime();
    Mode getMode();
    bool reset();
    float getBusVoltage_V(Channel channel);
    float getShuntVoltage_mV(Channel channel);
    float getCurrent_mA(Channel channel);
    float getPower_W(Channel channel);
    bool setCriticalShuntAlertLimit(Channel channel, float value);
    bool setWarningShuntAlertLimit(Channel channel, float value);
    float getCriticalShuntAlertLimit(Channel channel);
    float getWarningShuntAlertLimit(Channel channel);
    float getShuntVoltageSum_mV();
    bool setShuntVoltageSumLimit(float value);
    float getShuntVoltageSumLimit();
    float getCurrentSum_mA();
    bool setMaskEnable(bool summationChannelControl1, bool summationChannelControl2, bool summationChannelControl3, bool warningAlertLatchEnable, bool criticalAlertLatchEnable);
    bool isSummationChannelControlActive(Channel channel);
    bool isWarningAlertLatchEnable();
    bool isCriticalAlertLatchEnable();
    bool isCriticalAlertFlagActive(Channel channel);
    bool isSummationAlertFlag();
    bool isWarningAlertFlagActive(Channel channel);
    bool isPowerValidAlertFlag();
    bool isTimingControlAlertFlag();
    bool isConversionOk();
    bool setPowerValidUpperLimit(float value);
    bool setPowerValidLowerLimit(float value);
    float getPowerValidUpperLimit();
    float getPowerValidLowerLimit();
    uint16_t getManufacturerID();
    bool isManufacturerID();
    uint16_t getDieID();
    bool isDieID();

private:
    int handle;
    bool isInitialized;
    void swap(uint16_t &data);
    int16_t read16Swapped(uint8_t reg);
    bool write16Swapped(uint8_t reg, uint16_t value);
};

#endif // INA3221_H