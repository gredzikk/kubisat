#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib/BH1750.h"

// I2C defines
#define I2C_PORT i2c0

int main() {
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    sleep_ms(20000);
    // Initialize BH1750 sensor
    BH1750 sensor(BH1750_DEFAULT_ADDRESS);
    printf("init\n");
    if (!sensor.begin(CONTINUOUS_HIGH_RES_MODE)) {
        printf("BH1750 initialization failed!\n");
        return 1;
    }

    while (true) {
        float lux = sensor.readLightLevel();
        printf("Lux: %.2f\n", lux);
        sleep_ms(1000);
    }

    return 0;
}