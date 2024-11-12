#ifndef __BH1750_H__
#define __BH1750_H__

#include "hardware/i2c.h"

#define _BH1750_DEVICE_ID 0xE1  // Correct content of WHO_AM_I register

#define _BH1750_MTREG_MIN 31
#define _BH1750_MTREG_MAX 254
#define _BH1750_DEFAULT_MTREG 69

typedef enum {
    BH1750_DEFAULT_ADDRESS = 0x23,
    BH1750_ALTERNATIVE_ADDRESS = 0x5C
} bh1750_addr;

typedef enum {
    UNCONFIGURED_POWER_DOWN = 0x00,
	POWER_ON = 0x01,
    RESET = 0x07,
    CONTINUOUS_HIGH_RES_MODE = 0x10,
    CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
    CONTINUOUS_LOW_RES_MODE = 0x13,
    ONE_TIME_HIGH_RES_MODE = 0x20,
    ONE_TIME_HIGH_RES_MODE_2 = 0x21,
    ONE_TIME_LOW_RES_MODE = 0x23
} bh1750_mode;

class BH1750 {
public:
    BH1750(uint8_t addr = BH1750_DEFAULT_ADDRESS);
    bool begin(uint8_t mode = CONTINUOUS_HIGH_RES_MODE);
    void configure(uint8_t mode);
    float readLightLevel();

private:
    void write8(uint8_t data);
    uint8_t _i2c_addr;
};

#endif // __BH1750_H__