#include "BH1750.h"
#include "pico/stdlib.h"
#include <stdio.h>

BH1750::BH1750(uint8_t addr) {
    _i2c_addr = addr;
}

bool BH1750::begin(uint8_t mode) {
    write8(POWER_ON);
    write8(RESET);
    configure(mode);
    return true;
}

void BH1750::configure(uint8_t mode) {
    switch (mode) {
        case CONTINUOUS_HIGH_RES_MODE:
        case CONTINUOUS_HIGH_RES_MODE_2:
        case CONTINUOUS_LOW_RES_MODE:
        case ONE_TIME_HIGH_RES_MODE:
        case ONE_TIME_HIGH_RES_MODE_2:
        case ONE_TIME_LOW_RES_MODE:
            write8(mode);
            sleep_ms(10);
            break;
        default:
            printf("Invalid measurement mode\n");
            break;
    }
}

float BH1750::readLightLevel() {
    uint8_t buffer[2];
    i2c_read_blocking(i2c_default, _i2c_addr, buffer, 2, false);
    uint16_t level = (buffer[0] << 8) | buffer[1];
    
    float lux = static_cast<float>(level) / 1.2;
    return lux;
}

void BH1750::write8(uint8_t data) {
    uint8_t buf[1] = {data};
    i2c_write_blocking(i2c_default, _i2c_addr, buf, 1, false);
}