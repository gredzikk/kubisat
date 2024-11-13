#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

#include "ISensor.h"
#include "lib/BH1750/BH1750_WRAPPER.h"
#include "lib/BME280/BME280_WRAPPER.h"
#include "lib/DS3231/DS3231_WRAPPER.h"
#include "lib/INA3221/INA3221_WRAPPER.h"
#include "lib/LoRa/LoRa-RP2040.h"

#define I2C_PORT i2c0

int main()
{
    stdio_init_all();

    i2c_inst_t* i2c = i2c0;
    i2c_init(i2c, 400 * 1000);
    
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);    

    for (int i = 20; i > 0; --i) {
        std::cout << "Program starts in " << i << " seconds..." << std::endl;
        sleep_ms(1000);
    }

    auto& sensorManager = SensorWrapper::getInstance();

    // Initialize sensors
    sensorManager.initSensor(SensorType::LIGHT, i2c);
    sensorManager.initSensor(SensorType::ENVIRONMENT, i2c);

    // Read data
    while(true) {
        float lightLevel = sensorManager.readSensorData(SensorType::LIGHT, DataType::LIGHT_LEVEL);
        float temperature = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::TEMPERATURE);
        float humidity = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::HUMIDITY);
        float pressure = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::PRESSURE);


        printf("Light: %.2f lux\n", lightLevel);
        printf("Temperature: %.2f C\n", temperature);
        printf("Humidity: %.2f %%\n", humidity);
        sleep_ms(1000);
    }

    return 0;
}