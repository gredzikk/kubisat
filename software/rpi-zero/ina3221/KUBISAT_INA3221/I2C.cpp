#include <wiringPiI2C.h>
#include <list>
#include <unistd.h>
#include "i2c.h"

I2C::I2C() {
}

I2C::I2C(uint16_t deviceAddress) {
    init(deviceAddress);
}

void I2C::init(uint16_t deviceAddress) {
    handle = wiringPiI2CSetup(deviceAddress);
    isInitialized = true;
    if(handle == -1) {
        isInitialized = false;
    }
}

bool I2C::isInitialized(void) {
    return isInitialized;
}

int8_t I2C::read8(void) {
    return wiringPiI2CRead(handle);
}

bool I2C::write8(uint8_t data) {
    if(wiringPiI2CWrite(handle, data)) {
        return true;
    } else {
        return false;
    }
}

int8_t I2C::read8(uint8_t registerAddress) {
    return wiringPiI2CReadReg8(handle, registerAddress);
}

bool I2C::write8(uint8_t registerAddress, uint8_t data) {
    if(wiringPiI2CWriteReg8(handle, registerAddress, data)) {
        return true;
    } else {
        return false;
    }
}

int16_t I2C::read16(uint8_t registerAddress) {
    return wiringPiI2CReadReg16(handle, registerAddress);
}

bool I2C::write16(uint8_t registerAddress, uint16_t data) {
    if(wiringPiI2CWriteReg16(handle, registerAddress, data)) {
        return true;
    } else {
        return false;
    }
}
    
int16_t I2C::read16Swapped(uint8_t registerAddress) {
    i2cData data;
    data.uSData = wiringPiI2CReadReg16(handle, registerAddress);
    swap(data);
    return data.uSData;
}

bool I2C::write16Swapped(uint8_t registerAddress, uint16_t value) {
    i2cUData data;
    data.uSData = value;
    swapU(data);
    if(wiringPiI2CWriteReg16(handle, registerAddress, data.uSData)) {
        return true;
    } else {
        return false;
    }
}

std::list<int8_t> I2C::readList(uint8_t registerAddress, uint8_t numberOfValues) {
    std::list<int8_t> dataList;
    char registerBuffer[1] = {registerAddress};
    write(handle, registerBuffer, 1);
    char dataBuffer[numberOfValues] = {0};
    if(read(handle, dataBuffer, numberOfValues) != numberOfValues) {
        return dataList;
    }
    for(int index = 0; index != numberOfValues; index++) {
        dataList.push_back((int8_t)dataBuffer[index]);
    }

    return dataList;
}
    
void I2C::swap(i2cData &data) {
    int8_t temp = data.uCData[0];
    data.uCData[0] = data.uCData[1];
    data.uCData[1] = temp;
}
    
void I2C::swapU(i2cUData &data) {
    uint8_t temp = data.uCData[0];
    data.uCData[0] = data.uCData[1];
    data.uCData[1] = temp;
}
    
I2C::~I2C() {
}