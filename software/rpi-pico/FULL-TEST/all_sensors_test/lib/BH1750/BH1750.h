#ifndef __BH1750_H__
#define __BH1750_H__

#include "hardware/i2c.h"

// Define constants
#define _BH1750_DEVICE_ID 0xE1  // Correct content of WHO_AM_I register
#define _BH1750_MTREG_MIN 31
#define _BH1750_MTREG_MAX 254
#define _BH1750_DEFAULT_MTREG 69

class BH1750 {
public:
    // Scoped enum for measurement modes
    enum class Mode : uint8_t {
        UNCONFIGURED_POWER_DOWN = 0x00,
        POWER_ON = 0x01,
        RESET = 0x07,
        CONTINUOUS_HIGH_RES_MODE = 0x10,
        CONTINUOUS_HIGH_RES_MODE_2 = 0x11,
        CONTINUOUS_LOW_RES_MODE = 0x13,
        ONE_TIME_HIGH_RES_MODE = 0x20,
        ONE_TIME_HIGH_RES_MODE_2 = 0x21,
        ONE_TIME_LOW_RES_MODE = 0x23
    };

    BH1750(uint8_t addr = 0x23);
    bool begin(Mode mode = Mode::CONTINUOUS_HIGH_RES_MODE);
    void configure(Mode mode);
    float readLightLevel();

private:
    void write8(uint8_t data);
    uint8_t _i2c_addr;
};

#endif // __BH1750_H__