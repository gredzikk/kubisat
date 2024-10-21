#include "INA3221.h"

INA3221::INA3221() {
}

INA3221::INA3221(Address address) {
    init(address);
}

bool INA3221::init(Address address) {
    GestionI2C::init(address);
    return GestionI2C::isInitialize();
}

bool INA3221::setConfiguration(Channel1 channel1, Channel2 channel2, Channel3 channel3, Average average, 
                BusVoltageConversionTime busVoltageConversionTime, ShuntVoltageConversionTime shuntVoltageConversionTime, Mode mode) {
    return !write16Swapped(Config, channel1 | channel2 | channel3 | average | busVoltageConversionTime | shuntVoltageConversionTime | mode);
}

// *********************************
// Is channel 1 active?
// @Return true if yes, false if no
// *********************************
bool INA3221::isChannel1(void) {
    return read16Swapped(Config) & Channel1On;	
}

// *********************************
// Is channel 2 active?
// @Return true if yes, false if no
// *********************************
bool INA3221::isChannel2(void) {
    return read16Swapped(Config) & Channel2On;	
}
        
// *********************************
// Is channel 3 active?
// @Return true if yes, false if no
// *********************************
bool INA3221::isChannel3(void) {
    return read16Swapped(Config) & Channel3On;		
}

// *******************************************
// Get the number of measurements for averaging
// @Return the number of measurements for averaging
// *******************************************
Average INA3221::getAverage(void) {
    switch(read16Swapped(Config) & Avg_1024) {
        case Avg_1: return Avg_1;
        case Avg_4: return Avg_4;
        case Avg_16: return Avg_16;
        case Avg_64: return Avg_64;
        case Avg_128: return Avg_128;
        case Avg_256: return Avg_256;
        case Avg_512: return Avg_512;
        case Avg_1024: return Avg_1024;
    }
}

// *********************************************
// Get the Bus Voltage conversion time
// @Return the Bus Voltage conversion time
// *********************************************
BusVoltageConversionTime INA3221::getBusVoltageConversionTime(void) {
    switch(read16Swapped(Config) & VBUS_8244US) {
        case VBUS_140US: return VBUS_140US;
        case VBUS_204US: return VBUS_204US;
        case VBUS_332US: return VBUS_332US;
        case VBUS_588US: return VBUS_588US;
        case VBUS_1100US: return VBUS_1100US;
        case VBUS_2116US: return VBUS_2116US;
        case VBUS_4156US: return VBUS_4156US;
        case VBUS_8244US: return VBUS_8244US;
    }
}

// ***********************************************
// Get the Shunt Voltage conversion time
// @Return the Shunt Voltage conversion time
// ***********************************************
ShuntVoltageConversionTime INA3221::getShuntVoltageConversionTime(void) {
    switch(read16Swapped(Config) & SBUS_8244US) {
        case SBUS_140US: return SBUS_140US;
        case SBUS_204US: return SBUS_204US;
        case SBUS_332US: return SBUS_332US;
        case SBUS_588US: return SBUS_588US;
        case SBUS_1100US: return SBUS_1100US;
        case SBUS_2116US: return SBUS_2116US;
        case SBUS_4156US: return SBUS_4156US;
        case SBUS_8244US: return SBUS_8244US;
    }
}

// *************************
// Get the measurement mode
// @Return the measurement mode
// *************************
Mode INA3221::getMode(void) {
    switch(read16Swapped(Config) & ShuntAndBusVoltageContinuous) {
        case PowerDown: return PowerDown;
        case ShuntVoltageTriggered: return ShuntVoltageTriggered;
        case BusVoltageTrigerred: return BusVoltageTrigerred;
        case ShuntAndBusVoltageTriggered: return ShuntAndBusVoltageTriggered;
        case ShuntVoltageContinuous: return ShuntVoltageContinuous;
        case BusVoltageContinuous: return BusVoltageContinuous;
        case ShuntAndBusVoltageContinuous: return ShuntAndBusVoltageContinuous;
    }
}

// ***************
// Reset the module
// ***************
bool INA3221::reset(void) {
    return !write16Swapped(Config, RESET);
}

// *********************************************
// Get the Bus Voltage in Volts for a channel
// @Param the channel
// @Return the voltage in volts
// *********************************************
float INA3221::getBusVoltage_V(Channel channel) {
    while(!isConversionOk());
    switch(channel) {
        case Channel_1: return ((read16Swapped(Channel1BusVoltage) >> 3)) * _8MILLIVOLT;
        case Channel_2: return ((read16Swapped(Channel2BusVoltage) >> 3)) * _8MILLIVOLT;
        case Channel_3: return ((read16Swapped(Channel3BusVoltage) >> 3)) * _8MILLIVOLT;
    }
}

// **************************************************
// Get the Shunt Voltage in milliVolts for a channel
// @Param the channel
// @Return the voltage in milliVolts
// **************************************************
float INA3221::getShuntVoltage_mV(Channel channel) {
    while(!isConversionOk());
    switch(channel) {
        case Channel_1: return ((read16Swapped(Channel1ShuntVoltage) >> 3)) * _40MICROVOLT;
        case Channel_2: return ((read16Swapped(Channel2ShuntVoltage) >> 3)) * _40MICROVOLT;
        case Channel_3: return ((read16Swapped(Channel3ShuntVoltage) >> 3)) * _40MICROVOLT;
    }
}

// ************************************
// Get the current in mA for a channel
// @Param the channel
// @Return the current in mA
// ************************************
float INA3221::getCurrent_mA(Channel channel) {
    return getShuntVoltage_mV(channel) / ResistanceShunt;
}

// *************************************
// Get the power in W for a channel
// @Param the channel
// @Return the power in W
// *************************************
float INA3221::getPower_W(Channel channel) {
    return getBusVoltage_V(channel) * getCurrent_mA(channel) / 1000.0;
}

// **********************************************
// Set the critical shunt alert limit for a channel
// @Param the channel
// @Param the value in mV
// **********************************************
bool INA3221::setCriticalShuntAlertLimit(Channel channel, float value) {
    switch(channel) {
        case Channel_1: return !write16Swapped(Channel1CriticalAlertLimit, (uint16_t)(value / _40MICROVOLT) << 3);
        case Channel_2: return !write16Swapped(Channel2CriticalAlertLimit, (uint16_t)(value / _40MICROVOLT) << 3);
        case Channel_3: return !write16Swapped(Channel3CriticalAlertLimit, (uint16_t)(value / _40MICROVOLT) << 3);
    }
}

// **********************************************
// Set the warning shunt alert limit for a channel
// @Param the channel
// @Param the value in mV
// **********************************************
bool INA3221::setWarningShuntAlertLimit(Channel channel, float value) {
    switch(channel) {
        case Channel_1: return !write16Swapped(Channel1WarningAlertLimit, (uint16_t)(value / _40MICROVOLT) << 3);
        case Channel_2: return !write16Swapped(Channel2WarningAlertLimit, (uint16_t)(value / _40MICROVOLT) << 3);
        case Channel_3: return !write16Swapped(Channel3WarningAlertLimit, (uint16_t)(value / _40MICROVOLT) << 3);
    }	
}

// *****************************************
// Get the critical shunt alert limit for a channel
// @Param the channel
// @Return the value in mV
// *****************************************
float INA3221::getCriticalShuntAlertLimit(Channel channel) {
    switch(channel) {
        case Channel_1: return ((read16Swapped(Channel1CriticalAlertLimit) >> 3)) * _40MICROVOLT;
        case Channel_2: return ((read16Swapped(Channel2CriticalAlertLimit) >> 3)) * _40MICROVOLT;
        case Channel_3: return ((read16Swapped(Channel3CriticalAlertLimit) >> 3)) * _40MICROVOLT;
    }
}

// *****************************************
// Get the warning shunt alert limit for a channel
// @Param the channel
// @Return the value in mV
// *****************************************
float INA3221::getWarningShuntAlertLimit(Channel channel) {
    switch(channel) {
        case Channel_1: return ((read16Swapped(Channel1WarningAlertLimit) >> 3)) * _40MICROVOLT;
        case Channel_2: return ((read16Swapped(Channel2WarningAlertLimit) >> 3)) * _40MICROVOLT;
        case Channel_3: return ((read16Swapped(Channel3WarningAlertLimit) >> 3)) * _40MICROVOLT;
    }
}

// ******************************************
// Get the sum of shunt voltages
// @Return the sum of shunt voltages in mV
// ******************************************
float INA3221::getShuntVoltageSum_mV(void) {
    while(!isConversionOk());
    return ((read16Swapped(ShuntVoltageSum) >> 1)) * _40MICROVOLT;
}

// *********************************************************
// Set the shunt voltage sum limit
// @Param value in mV
// *********************************************************
bool INA3221::setShuntVoltageSumLimit(float value) {
    return !write16Swapped(ShuntVoltageSumLimit, (uint16_t)(value / 0.04) << 1);
}

// ****************************************************
// Get the shunt voltage sum limit
// @Return value in mV
// ****************************************************
float INA3221::getShuntVoltageSumLimit(void) {
    return ((read16Swapped(ShuntVoltageSumLimit) >> 1)) * _40MICROVOLT;
}

// ******************************************
// Get the sum of currents on the shunts
// @Return the sum of currents in mA
// ******************************************
float INA3221::getCurrentSum_mA(void) {
    return getShuntVoltageSum_mV() / ResistanceShunt;
}

// ***************************************************************
// Set the mask
// @Param Enable channel1 for shunt voltage summation
// @Param Enable channel2 for shunt voltage summation
// @Param Enable channel3 for shunt voltage summation
// @Param Enable the warning pin
// @Param Enable the critical alert pin
// ***************************************************************
bool INA3221::setMaskEnable(SummationChannelControl1 summationChannelControl1, SummationChannelControl2 summationChannelControl2, 
            SummationChannelControl3 summationChannelControl3, WarningAlertLatchEnable warningAlertLatchEnable, 
            CriticalAlertLatchEnable criticalAlertLatchEnable) {
    return !write16Swapped(MaskEnable, summationChannelControl1 | summationChannelControl2 | summationChannelControl3 | warningAlertLatchEnable | criticalAlertLatchEnable);
}

// *************************************************************
// Does channel1 participate in shunt voltage summation?
// @Return true if yes, false if no
// *************************************************************
bool INA3221::isSummationChannelControl1(void) {
    return read16Swapped(MaskEnable) & SummationChannelControl1On;	
}

// *************************************************************
// Does channel2 participate in shunt voltage summation?
// @Return true if yes, false if no
// *************************************************************
bool INA3221::isSummationChannelControl2(void) {
    return read16Swapped(MaskEnable) & SummationChannelControl2On;		
}

// *************************************************************
// Does channel3 participate in shunt voltage summation?
// @Return true if yes, false if no
// *************************************************************
bool INA3221::isSummationChannelControl3(void) {
    return read16Swapped(MaskEnable) & SummationChannelControl3On;	
}

// ******************************************
// Is WarningAlertLatchEnable active?
// @Return true if yes, false if no
// ******************************************
bool INA3221::isWarningAlertLatchEnable(void) {
    return read16Swapped(MaskEnable) & WarningAlertLatchEnableOn;	
}

// ******************************************
// Is CriticalAlertLatchEnable active?
// @Return true if yes, false if no
// ******************************************
bool INA3221::isCriticalAlertLatchEnable(void) {
    return read16Swapped(MaskEnable) & CriticalAlertLatchEnableOn;	
}

// ********************************************
// Is CriticalAlertFlag active for channel1?
// @Return true if yes, false if no
// ********************************************
bool INA3221::isCriticalAlertFlagIndicator1(void) {
    return read16Swapped(MaskEnable) & CriticalAlertFlagIndicator1On;	
}

// ********************************************
// Is CriticalAlertFlag active for channel2?
// @Return true if yes, false if no
// ********************************************
bool INA3221::isCriticalAlertFlagIndicator2(void) {
    return read16Swapped(MaskEnable) & CriticalAlertFlagIndicator2On;	
}

// ********************************************
// Is CriticalAlertFlag active for channel3?
// @Return true if yes, false if no
// ********************************************
bool INA3221::isCriticalAlertFlagIndicator3(void) {
    return read16Swapped(MaskEnable) & CriticalAlertFlagIndicator3On;	
}

// ************************************
// Is SummationAlertFlag active?
// @Return true if yes, false if no
// ************************************
bool INA3221::isSummationAlertFlag(void) {
    return read16Swapped(MaskEnable) & SummationAlertFlagOn;	
}

// *******************************************
// Is WarningAlertFlag active for channel1?
// @Return true if yes, false if no
// *******************************************
bool INA3221::isWarningAlertFlagIndicator1(void) {
    return read16Swapped(MaskEnable) & WarningAlertFlagIndicator1On;	
}

// *******************************************
// Is WarningAlertFlag active for channel2?
// @Return true if yes, false if no
// *******************************************
bool INA3221::isWarningAlertFlagIndicator2(void) {
    return read16Swapped(MaskEnable) & WarningAlertFlagIndicator2On;	
}

// *******************************************
// Is WarningAlertFlag active for channel3?
// @Return true if yes, false if no
// *******************************************
bool INA3221::isWarningAlertFlagIndicator3(void) {
    return read16Swapped(MaskEnable) & WarningAlertFlagIndicator3On;	
}

// ************************************************************************************
// Read the PowerValidAlertFlag
// @Return true if all bus voltages are above the powerValidUpperLimit
//         false if any bus voltage is below the powerValidLowerLimit
// ************************************************************************************
bool INA3221::isPowerValidAlertFlag(void) {
    return read16Swapped(MaskEnable) & PowerValidAlertFlagOn;	
}

// ****************************************
// Is TimingControlAlertFlag active?
// @Return false if ok, true if not
// ****************************************
bool INA3221::isTimingControlAlertFlag(void) {
    return !(read16Swapped(MaskEnable) & TimingControlAlertFlagOn);	
}

// ******************************************
// Is the first conversion in progress?
// @Return true if yes, false if no
// ******************************************
bool INA3221::isConversionOk(void) {
    return read16Swapped(MaskEnable) & ConversionWriting;	
}

// *************************************************************************************************
// Set the high comparison value for bus voltages to activate the PowerValidAlertFlag
// @Param high comparison value for bus voltages
// *************************************************************************************************
bool INA3221::setPowerValidUpperLimit(float value) {
    return !write16Swapped(PowerValidUpperLimit, (uint16_t)(value / _8MILLIVOLT) << 3);
}

// ******************************************************************************************************
// Set the low comparison value for each bus voltage to activate the PowerValidAlertFlag
// @Param low comparison value for bus voltages
// ******************************************************************************************************
bool INA3221::setPowerValidLowerLimit(float value) {
    return !write16Swapped(PowerValidLowerLimit, (uint16_t)(value / _8MILLIVOLT) << 3);
}

// ************************************************************
// Get the high comparison value for bus voltages
// @Return high comparison value for bus voltages in V
// ************************************************************
float INA3221::getPowerValidUpperLimit(void) {
    return ((read16Swapped(PowerValidUpperLimit) >> 3)) * _8MILLIVOLT;
}

// ************************************************************
// Get the low comparison value for bus voltages
// @Return low comparison value for bus voltages in V
// ************************************************************
float INA3221::getPowerValidLowerLimit(void) {
    return ((read16Swapped(PowerValidLowerLimit) >> 3)) * _8MILLIVOLT;
}

// **********************************************************
// Get the Manufacturer ID (TI for Texas Instrument 0x5449)
// @Return the manufacturer ID
// **********************************************************
uint16_t INA3221::getManufacturerID(void) {
    return read16Swapped(ManufacturerID);
}

// ***************************************
// Is the Manufacturer ID TI (0x5449)?
// @Return true if yes, false if no
// ***************************************
bool INA3221::isManufacturerID(void) {
    return !(getManufacturerID() & ~TI);
}

// *********************************
// Get the device ID (0x3220)
// @Return the device ID
// *********************************
uint16_t INA3221::getDieID(void) {
    return read16Swapped(DieID);
}

// **********************************
// Is the device ID 0x3220?
// @Return true if yes, false if no
// **********************************
bool INA3221::isDieID(void) {
    return !(getDieID() & ~INA3221);
}