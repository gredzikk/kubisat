#include "INA3221.h"

/**
 * @brief Default constructor for INA3221 class.
 */
INA3221::INA3221() : isInitialized(false) {
}

/**
 * @brief Constructor for INA3221 class with I2C address.
 * @param address I2C address of the INA3221 device.
 */
INA3221::INA3221(Address address) {
    init(address);
}

/**
 * @brief Initializes the INA3221 device with the given I2C address.
 * @param address I2C address of the INA3221 device.
 * @return True if initialization is successful, false otherwise.
 */
bool INA3221::init(Address address) {
    handle = wiringPiI2CSetup(address);
    isInitialized = (handle != -1);
    return isInitialized;
}

/**
 * @brief Sets the configuration for the INA3221 device.
 * @param channel1 Configuration for channel 1.
 * @param channel2 Configuration for channel 2.
 * @param channel3 Configuration for channel 3.
 * @param average Averaging mode.
 * @param busVoltageConversionTime Bus voltage conversion time.
 * @param shuntVoltageConversionTime Shunt voltage conversion time.
 * @param mode Operating mode.
 * @return True if configuration is successful, false otherwise.
 */
bool INA3221::setConfiguration(Channel channel1, Channel channel2, Channel channel3, Average average, 
                               BusVoltageConversionTime busVoltageConversionTime, ShuntVoltageConversionTime shuntVoltageConversionTime, Mode mode) {
    uint16_t config = channel1 | channel2 | channel3 | average | busVoltageConversionTime | shuntVoltageConversionTime | mode;
    return write16Swapped(0x00, config);
}

/**
 * @brief Checks if a specific channel is active.
 * @param channel Channel to check.
 * @return True if the channel is active, false otherwise.
 */
bool INA3221::isChannelActive(Channel channel) {
    uint16_t config = read16Swapped(0x00);
    return config & (1 << channel);
}

/**
 * @brief Gets the averaging mode.
 * @return Averaging mode.
 */
INA3221::Average INA3221::getAverage() {
    uint16_t config = read16Swapped(0x00);
    return static_cast<Average>(config & 0x0E00);
}

/**
 * @brief Gets the bus voltage conversion time.
 * @return Bus voltage conversion time.
 */
INA3221::BusVoltageConversionTime INA3221::getBusVoltageConversionTime() {
    uint16_t config = read16Swapped(0x00);
    return static_cast<BusVoltageConversionTime>(config & 0x01C0);
}

/**
 * @brief Gets the shunt voltage conversion time.
 * @return Shunt voltage conversion time.
 */
INA3221::ShuntVoltageConversionTime INA3221::getShuntVoltageConversionTime() {
    uint16_t config = read16Swapped(0x00);
    return static_cast<ShuntVoltageConversionTime>(config & 0x0038);
}

/**
 * @brief Gets the operating mode.
 * @return Operating mode.
 */
INA3221::Mode INA3221::getMode() {
    uint16_t config = read16Swapped(0x00);
    return static_cast<Mode>(config & 0x0007);
}

/**
 * @brief Resets the INA3221 device.
 * @return True if reset is successful, false otherwise.
 */
bool INA3221::reset() {
    return write16Swapped(0x00, 0x8000);
}

/**
 * @brief Gets the bus voltage in volts for a specific channel.
 * @param channel Channel to read.
 * @return Bus voltage in volts.
 */
float INA3221::getBusVoltage_V(Channel channel) {
    while (!isConversionOk());
    uint16_t rawValue = read16Swapped(0x02 + channel * 2);
    return (rawValue >> 3) * 0.008;
}

/**
 * @brief Gets the shunt voltage in millivolts for a specific channel.
 * @param channel Channel to read.
 * @return Shunt voltage in millivolts.
 */
float INA3221::getShuntVoltage_mV(Channel channel) {
    while (!isConversionOk());
    uint16_t rawValue = read16Swapped(0x01 + channel * 2);
    return (rawValue >> 3) * 0.04;
}

/**
 * @brief Gets the current in milliamps for a specific channel.
 * @param channel Channel to read.
 * @return Current in milliamps.
 */
float INA3221::getCurrent_mA(Channel channel) {
    return getShuntVoltage_mV(channel) / 0.01; // Assuming a shunt resistor of 0.01 ohms
}

/**
 * @brief Gets the power in watts for a specific channel.
 * @param channel Channel to read.
 * @return Power in watts.
 */
float INA3221::getPower_W(Channel channel) {
    return getBusVoltage_V(channel) * getCurrent_mA(channel) / 1000.0;
}

/**
 * @brief Sets the critical shunt alert limit for a specific channel.
 * @param channel Channel to set.
 * @param value Alert limit in millivolts.
 * @return True if successful, false otherwise.
 */
bool INA3221::setCriticalShuntAlertLimit(Channel channel, float value) {
    uint16_t limit = static_cast<uint16_t>(value / 0.04) << 3;
    return write16Swapped(0x07 + channel * 2, limit);
}

/**
 * @brief Sets the warning shunt alert limit for a specific channel.
 * @param channel Channel to set.
 * @param value Alert limit in millivolts.
 * @return True if successful, false otherwise.
 */
bool INA3221::setWarningShuntAlertLimit(Channel channel, float value) {
    uint16_t limit = static_cast<uint16_t>(value / 0.04) << 3;
    return write16Swapped(0x08 + channel * 2, limit);
}

/**
 * @brief Gets the critical shunt alert limit for a specific channel.
 * @param channel Channel to read.
 * @return Critical shunt alert limit in millivolts.
 */
float INA3221::getCriticalShuntAlertLimit(Channel channel) {
    uint16_t rawValue = read16Swapped(0x07 + channel * 2);
    return (rawValue >> 3) * 0.04;
}

/**
 * @brief Gets the warning shunt alert limit for a specific channel.
 * @param channel Channel to read.
 * @return Warning shunt alert limit in millivolts.
 */
float INA3221::getWarningShuntAlertLimit(Channel channel) {
    uint16_t rawValue = read16Swapped(0x08 + channel * 2);
    return (rawValue >> 3) * 0.04;
}

/**
 * @brief Gets the sum of shunt voltages in millivolts.
 * @return Sum of shunt voltages in millivolts.
 */
float INA3221::getShuntVoltageSum_mV() {
    while (!isConversionOk());
    uint16_t rawValue = read16Swapped(0x0E);
    return (rawValue >> 1) * 0.04;
}

/**
 * @brief Sets the shunt voltage sum limit.
 * @param value Limit in millivolts.
 * @return True if successful, false otherwise.
 */
bool INA3221::setShuntVoltageSumLimit(float value) {
    uint16_t limit = static_cast<uint16_t>(value / 0.04) << 1;
    return write16Swapped(0x0F, limit);
}

/**
 * @brief Gets the shunt voltage sum limit.
 * @return Shunt voltage sum limit in millivolts.
 */
float INA3221::getShuntVoltageSumLimit() {
    uint16_t rawValue = read16Swapped(0x0F);
    return (rawValue >> 1) * 0.04;
}

/**
 * @brief Gets the sum of currents in milliamps.
 * @return Sum of currents in milliamps.
 */
float INA3221::getCurrentSum_mA() {
    return getShuntVoltageSum_mV() / 0.01; // Assuming a shunt resistor of 0.01 ohms
}

/**
 * @brief Sets the mask enable register.
 * @param summationChannelControl1 Enable summation for channel 1.
 * @param summationChannelControl2 Enable summation for channel 2.
 * @param summationChannelControl3 Enable summation for channel 3.
 * @param warningAlertLatchEnable Enable warning alert latch.
 * @param criticalAlertLatchEnable Enable critical alert latch.
 * @return True if successful, false otherwise.
 */
bool INA3221::setMaskEnable(bool summationChannelControl1, bool summationChannelControl2, bool summationChannelControl3, bool warningAlertLatchEnable, bool criticalAlertLatchEnable) {
    uint16_t mask = (summationChannelControl1 << 12) | (summationChannelControl2 << 11) | (summationChannelControl3 << 10) | (warningAlertLatchEnable << 9) | (criticalAlertLatchEnable << 8);
    return write16Swapped(0x06, mask);
}

/**
 * @brief Checks if summation channel control is active for a specific channel.
 * @param channel Channel to check.
 * @return True if active, false otherwise.
 */
bool INA3221::isSummationChannelControlActive(Channel channel) {
    uint16_t mask = read16Swapped(0x06);
    return mask & (1 << (12 - channel));
}

/**
 * @brief Checks if warning alert latch is enabled.
 * @return True if enabled, false otherwise.
 */
bool INA3221::isWarningAlertLatchEnable() {
    uint16_t mask = read16Swapped(0x06);
    return mask & 0x0200;
}

/**
 * @brief Checks if critical alert latch is enabled.
 * @return True if enabled, false otherwise.
 */
bool INA3221::isCriticalAlertLatchEnable() {
    uint16_t mask = read16Swapped(0x06);
    return mask & 0x0100;
}

/**
 * @brief Checks if critical alert flag is active for a specific channel.
 * @param channel Channel to check.
 * @return True if active, false otherwise.
 */
bool INA3221::isCriticalAlertFlagActive(Channel channel) {
    uint16_t mask = read16Swapped(0x06);
    return mask & (1 << (3 - channel));
}

/**
 * @brief Checks if summation alert flag is active.
 * @return True if active, false otherwise.
 */
bool INA3221::isSummationAlertFlag() {
    uint16_t mask = read16Swapped(0x06);
    return mask & 0x0008;
}

/**
 * @brief Checks if warning alert flag is active for a specific channel.
 * @param channel Channel to check.
 * @return True if active, false otherwise.
 */
bool INA3221::isWarningAlertFlagActive(Channel channel) {
    uint16_t mask = read16Swapped(0x06);
    return mask & (1 << (6 - channel));
}

/**
 * @brief Checks if power valid alert flag is active.
 * @return True if active, false otherwise.
 */
bool INA3221::isPowerValidAlertFlag() {
    uint16_t mask = read16Swapped(0x06);
    return mask & 0x0002;
}

/**
 * @brief Checks if timing control alert flag is active.
 * @return True if active, false otherwise.
 */
bool INA3221::isTimingControlAlertFlag() {
    uint16_t mask = read16Swapped(0x06);
    return !(mask & 0x0001);
}

/**
 * @brief Checks if conversion is complete.
 * @return True if complete, false otherwise.
 */
bool INA3221::isConversionOk() {
    std::cout << "conversion ok called" << std::endl;
    uint16_t mask = read16Swapped(0x06);
    return mask & 0x8000;
}

/**
 * @brief Sets the power valid upper limit.
 * @param value Upper limit in volts.
 * @return True if successful, false otherwise.
 */
bool INA3221::setPowerValidUpperLimit(float value) {
    uint16_t limit = static_cast<uint16_t>(value / 0.008) << 3;
    return write16Swapped(0x03, limit);
}

/**
 * @brief Sets the power valid lower limit.
 * @param value Lower limit in volts.
 * @return True if successful, false otherwise.
 */
bool INA3221::setPowerValidLowerLimit(float value) {
    uint16_t limit = static_cast<uint16_t>(value / 0.008) << 3;
    return write16Swapped(0x04, limit);
}

/**
 * @brief Gets the power valid upper limit.
 * @return Upper limit in volts.
 */
float INA3221::getPowerValidUpperLimit() {
    uint16_t rawValue = read16Swapped(0x03);
    return (rawValue >> 3) * 0.008;
}

/**
 * @brief Gets the power valid lower limit.
 * @return Lower limit in volts.
 */
float INA3221::getPowerValidLowerLimit() {
    uint16_t rawValue = read16Swapped(0x04);
    return (rawValue >> 3) * 0.008;
}

/**
 * @brief Gets the manufacturer ID.
 * @return Manufacturer ID.
 */
uint16_t INA3221::getManufacturerID() {
    return read16Swapped(0xFE);
}

/**
 * @brief Checks if the manufacturer ID is correct.
 * @return True if correct, false otherwise.
 */
bool INA3221::isManufacturerID() {
    return getManufacturerID() == 0x5449;
}

/**
 * @brief Gets the device ID.
 * @return Device ID.
 */
uint16_t INA3221::getDieID() {
    return read16Swapped(0xFF);
}

/**
 * @brief Checks if the device ID is correct.
 * @return True if correct, false otherwise.
 */
bool INA3221::isDieID() {
    return getDieID() == 0x3220;
}

/**
 * @brief Swaps the byte order of a 16-bit value.
 * @param data Value to swap.
 */
void INA3221::swap(uint16_t &data) {
    data = (data >> 8) | (data << 8);
}

/**
 * @brief Reads a 16-bit value from a register with swapped byte order.
 * @param reg Register address.
 * @return 16-bit value.
 */
int16_t INA3221::read16Swapped(uint8_t reg) {
    uint16_t data = wiringPiI2CReadReg16(handle, reg);
    swap(data);
    return data;
}

/**
 * @brief Writes a 16-bit value to a register with swapped byte order.
 * @param reg Register address.
 * @param value Value to write.
 * @return True if successful, false otherwise.
 */
bool INA3221::write16Swapped(uint8_t reg, uint16_t value) {
    swap(value);
    return wiringPiI2CWriteReg16(handle, reg, value) == 0;
}