// main.cpp
int main() {
    // Initialize I2C
    i2c_inst_t* i2c = i2c0;
    i2c_init(i2c, 100000);

    auto& sensorManager = SensorWrapper::getInstance();

    // Initialize sensors
    sensorManager.initSensor(SensorType::LIGHT, i2c);
    sensorManager.initSensor(SensorType::ENVIRONMENT, i2c);

    // Read data
    float lightLevel = sensorManager.readSensorData(SensorType::LIGHT, DataType::LIGHT_LEVEL);
    float temperature = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::TEMPERATURE);
    float humidity = sensorManager.readSensorData(SensorType::ENVIRONMENT, DataType::HUMIDITY);

    printf("Light: %.2f lux\n", lightLevel);
    printf("Temperature: %.2f C\n", temperature);
    printf("Humidity: %.2f %%\n", humidity);

    return 0;
}
