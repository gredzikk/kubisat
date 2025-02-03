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
#include <iostream>
#include <iomanip>

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

    sensorManager.initSensor(SensorType::LIGHT, i2c);
    sensorManager.initSensor(SensorType::ENVIRONMENT, i2c);
    sensorManager.initSensor(SensorType::POWER, i2c);

    std::map<std::string, std::string> powerConfig = {
        {"operating_mode", "continuous"},  
        {"shunt_measure", "enable"},     
        {"bus_measure", "enable"},       
        {"averaging_mode", "16"},         
    };

    sensorManager.configureSensor(SensorType::POWER, powerConfig);

    while(true) {
        float lightLevel = sensorManager.readSensorData(SensorType::LIGHT, DataType::LIGHT_LEVEL);
        float temperature = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::TEMPERATURE);
        float humidity = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::HUMIDITY);
        float pressure = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::PRESSURE);

        float voltage_ch1 = sensorManager.readSensorData(SensorType::POWER, DataType::VOLTAGE_CH1);
        float voltage_ch2 = sensorManager.readSensorData(SensorType::POWER, DataType::VOLTAGE_CH2);
        float voltage_ch3 = sensorManager.readSensorData(SensorType::POWER, DataType::VOLTAGE_CH3);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Light: " << lightLevel << " lux" << std::endl;
        std::cout << "Temperature: " << temperature << " C" << std::endl; 
        std::cout << "Humidity: " << humidity << " %" << std::endl;
        
        std::cout << "Voltages: 1: " << voltage_ch1 << "V, 2: " << voltage_ch2 << "V, 3: " << voltage_ch3 << "V" << std::endl;
        sleep_ms(1000);
    }

    return 0;
}