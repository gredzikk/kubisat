#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "Beastdevices_INA3221.h"
#include "BMP280.h" // Include BMP280 header
#include <iostream>
#include <iomanip>
#include <sstream>
#include <deque>
#include <string>

// Constants
const float BATTERY_CAPACITY_MAH = 2000.0; // Battery capacity in mAh
const int AVERAGE_WINDOW_SIZE = 10;         // Number of measurements to average
const float TIME_INTERVAL_HOURS = 1.0 / 3600.0; // Time interval in hours (1 second)

// I2C defines
#define I2C_PORT i2c0

struct INA3221Data {
    float batteryVoltage;
    float usbCurrent;
    float solarCurrent;
    float railVoltage;
    float railCurrent;
    float chargingCurrent;
    float totalCurrent;
    float totalMahDrained;
};

struct BMP280Data {
    float temperature; // in Celsius
    float pressure;    // in kPa
    float humidity;    // in %RH
};

struct DataLog {
    INA3221Data ina3221Data;
    BMP280Data bmp280Data;
};

// Function Prototypes
INA3221Data collectINA3221Data(INA3221 &ina3221, float &totalMahDrained, std::deque<float> &timeRemainingHistory);
BMP280Data collectBMP280Data(BMP280 &bmp280);
void printINA3221Data(const INA3221Data &data);
void printBMP280Data(const BMP280Data &data);
void printData(const INA3221Data &inaData, const BMP280Data &bmpData);

int main()
{
    stdio_init_all();

    // Initialize I2C interface
    i2c_init(I2C_PORT, 400*1000); // Initialize I2C at 400kHz
    
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    for (int i = 20; i > 0; --i) {
    std::cout << "Program starts in " << i << " seconds..." << std::endl;
    sleep_ms(1000);
}

    // Initialize INA3221
    INA3221 ina3221(INA3221_ADDR40_GND);
    ina3221.begin();
    ina3221.setModeContinious();
    ina3221.setBusMeasEnable();
    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);
    
    printf( "Initialized INA3221 sensor.\n");

    // Initialize BMP280
    BMP280 bmp280(I2C_PORT, BMP280::ADDR_SDO_LOW);
    if (!bmp280.init()) {
        printf( "Failed to initialize BMP280 sensor.\n");
        return -1;
    }
    printf( "Initialized BMP280 sensor.\n");

    // Data Structures for Logging
    std::deque<float> timeRemainingHistory;
    float totalMahDrained = 0.0;
    DataLog collectedData;

    printf( "Entering main loop...\n");

    while (true) {
        // Collect Data from INA3221
        INA3221Data inaData = collectINA3221Data(ina3221, totalMahDrained, timeRemainingHistory);
        
        // Collect Data from BMP280
        BMP280Data bmpData = collectBMP280Data(bmp280);

        // Print the data
        printData(inaData, bmpData);

        // Optional: Log data to UART or file as needed

        sleep_ms(250); // Sleep for 250 milliseconds
    }

    return 0;
}

// Function Definitions

INA3221Data collectINA3221Data(INA3221 &ina3221, float &totalMahDrained, std::deque<float> &timeRemainingHistory) {
    INA3221Data data;

    data.batteryVoltage = ina3221.getVoltage(INA3221_CH1);
    data.usbCurrent = ina3221.getCurrent_mA(INA3221_CH1);
    data.solarCurrent = ina3221.getCurrent_mA(INA3221_CH3);
    data.railVoltage = ina3221.getVoltage(INA3221_CH2);
    data.railCurrent = ina3221.getCurrent_mA(INA3221_CH2);

    data.chargingCurrent = data.usbCurrent + data.solarCurrent;
    data.totalCurrent = data.chargingCurrent - data.railCurrent;

    // Calculate Time Remaining
    float timeRemainingHours = 0.0;
    if (data.totalCurrent < 0) {
        timeRemainingHours = BATTERY_CAPACITY_MAH / -data.totalCurrent;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << timeRemainingHours;
        // You can store this string if needed
    }

    // Calculate Total mAh Drained
    float mahDrainedLastInterval = -data.totalCurrent * TIME_INTERVAL_HOURS;
    totalMahDrained += mahDrainedLastInterval;
    data.totalMahDrained = totalMahDrained;

    // Manage Time Remaining History (Optional)
    if (data.totalCurrent < 0) {
        if (timeRemainingHistory.size() >= AVERAGE_WINDOW_SIZE) {
            timeRemainingHistory.pop_front();
        }
        timeRemainingHistory.push_back(timeRemainingHours);
    }

    // Calculate Average Time Remaining (Optional)
    // This can be handled separately if needed

    return data;
}

BMP280Data collectBMP280Data(BMP280 &bmp280) {
    BMP280Data data;

    int32_t raw_temperature, raw_pressure, raw_humidity;
    if (bmp280.read_raw_all(&raw_temperature, &raw_pressure, &raw_humidity)) {
        data.temperature = bmp280.convert_temperature(raw_temperature);
        data.pressure = bmp280.convert_pressure(raw_pressure);
        data.humidity = bmp280.convert_humidity(raw_humidity);
    } else {
        data.temperature = 0.0f; // Handle error as needed
        data.pressure = 0.0f;    // Handle error as needed
        data.humidity = 0.0f;    // Handle error as needed
    }

    return data;
}

void printINA3221Data(const INA3221Data &data) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Battery Voltage: " << data.batteryVoltage << " V\n";
    std::cout << "USB Current: " << data.usbCurrent << " mA\n";
    std::cout << "Solar Current: " << data.solarCurrent << " mA\n";
    std::cout << "Rail Voltage: " << data.railVoltage << " V\n";
    std::cout << "Rail Current: " << data.railCurrent << " mA\n";
    std::cout << "Charging Current: " << data.chargingCurrent << " mA\n";
    std::cout << "Total Current: " << data.totalCurrent << " mA\n";
    std::cout << "Total mAh Drained: " << data.totalMahDrained << " mAh\n";
}

void printBMP280Data(const BMP280Data &data) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Temperature: " << data.temperature << " C\n";
    std::cout << "Pressure: " << data.pressure << " kPa\n";
    std::cout << "Humidity: " << data.humidity << " %RH\n";
}

void printData(const INA3221Data &inaData, const BMP280Data &bmpData) {
    printINA3221Data(inaData);
    printBMP280Data(bmpData);

    // Clear the terminal screen and move the cursor to top-left
    std::cout << "\033[2J\033[H";
}