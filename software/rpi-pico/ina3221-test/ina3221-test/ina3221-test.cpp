#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "Beastdevices_INA3221.h"
#include <iostream>
#include <unistd.h>
#include <iomanip> // For std::setw, std::setprecision, and std::fixed
#include <sstream> // For std::ostringstream
#include <deque>   // For std::deque
#include <fstream> // For file operations

const float BATTERY_CAPACITY_MAH = 2000.0; // Battery capacity in mAh
const int AVERAGE_WINDOW_SIZE = 10;         // Number of measurements to average
const float BATTERY_MIN_VOLTAGE = 2.4;
const float BATTERY_MAX_VOLTAGE = 4.2;
const float TIME_INTERVAL_HOURS = 1.0 / 3600.0; // Time interval in hours (1 second)

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart0
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 12
#define UART_RX_PIN 13

struct DataLog {
    float batteryVoltage;
    float usbCurrent;
    float solarCurrent;
    float railVoltage;
    float railCurrent;
    float chargingCurrent;
    float totalCurrent;
    std::string timeRemaining;
    float averageTimeRemaining;
    float totalMahDrained;
};

DataLog collectData(INA3221 &ina3221, float &totalMahDrained, std::deque<float> &timeRemainingHistory) {
    DataLog data;

    data.batteryVoltage = ina3221.getVoltage(INA3221_CH1);
    data.usbCurrent = ina3221.getCurrent_mA(INA3221_CH1);
    data.solarCurrent = ina3221.getCurrent_mA(INA3221_CH3);
    data.railVoltage = ina3221.getVoltage(INA3221_CH2);
    data.railCurrent = ina3221.getCurrent_mA(INA3221_CH2);

    data.chargingCurrent = data.usbCurrent + data.solarCurrent;
    data.totalCurrent = data.chargingCurrent - data.railCurrent;

    float timeRemainingHours = 0.0;
    if (data.totalCurrent < 0) {
        timeRemainingHours = BATTERY_CAPACITY_MAH / -data.totalCurrent;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << timeRemainingHours;
        data.timeRemaining = oss.str();
    } else {
        data.timeRemaining = "-1";
    }

    if (data.totalCurrent < 0) {
        if (timeRemainingHistory.size() >= AVERAGE_WINDOW_SIZE) {
            timeRemainingHistory.pop_front();
        }
        timeRemainingHistory.push_back(timeRemainingHours);
    }

    data.averageTimeRemaining = 0.0;
    if (!timeRemainingHistory.empty()) {
        float sum = 0.0;
        for (float t : timeRemainingHistory) {
            sum += t;
        }
        data.averageTimeRemaining = sum / timeRemainingHistory.size();
    }

    float mahDrainedLastSecond = -data.totalCurrent * TIME_INTERVAL_HOURS;
    totalMahDrained += mahDrainedLastSecond;

    data.totalMahDrained = totalMahDrained;

    return data;
}

void printData(const DataLog &data) {
    std::cout << "Battery Voltage: " << data.batteryVoltage << " V\n";
    std::cout << "USB Current: " << data.usbCurrent << " mA\n";
    std::cout << "Solar Current: " << data.solarCurrent << " mA\n";
    std::cout << "Rail Voltage: " << data.railVoltage << " V\n";
    std::cout << "Rail Current: " << data.railCurrent << " mA\n";
    std::cout << "Charging Current: " << data.chargingCurrent << " mA\n";
    std::cout << "Total Current: " << data.totalCurrent << " mA\n";
    std::cout << "Time Remaining: " << data.timeRemaining << " h\n";
    std::cout << "Average Time Remaining: " << data.averageTimeRemaining << " h\n";
    std::cout << "Total mAh Drained: " << data.totalMahDrained << " mAh\n";

    // Clear the terminal screen and move the cursor to top-left
    std::cout << "\033[2J\033[H";
}

int main()
{
    stdio_init_all();

    // Initialize I2C interface
    i2c_init(i2c_default, 400*1000); // Initialize I2C at 400kHz
    
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    uart_puts(UART_ID, "inited uart\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    INA3221 ina3221(INA3221_ADDR40_GND);

    uart_puts(UART_ID, "ina instance created\n");

    // Initialize the INA3221 device
    ina3221.begin();
    uart_puts(UART_ID, "ina begin\n");

    // Set the device to continuous mode
    ina3221.setModeContinious();
    uart_puts(UART_ID, "mode set\n");


    // Set power valid upper and lower limits
    ina3221.setPwrValidUpLimit(5500);
    ina3221.setPwrValidLowLimit(3300);

    ina3221.setBusMeasEnable();

    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);

    uart_puts(UART_ID, "avg mode set\n");


    std::deque<float> timeRemainingHistory;
    float totalMahDrained = 0.0;
    int lineCounter = 0;
    std::string collectedData;

    uart_puts(UART_ID, "entering main loop\n");

    while (true) {
        DataLog data = collectData(ina3221, totalMahDrained, timeRemainingHistory);

        // Print the data
        printData(data);

        // Collect data in semicolon-separated format
        // std::ostringstream dataStream;
        // dataStream << data.batteryVoltage << ";" << data.usbCurrent << ";" << data.solarCurrent << ";" 
        //            << data.railVoltage << ";" << data.railCurrent << ";" << data.chargingCurrent << ";" 
        //            << data.totalCurrent << ";" << data.timeRemaining << ";" << data.averageTimeRemaining << ";" 
        //            << data.totalMahDrained << "\n";
        // collectedData += dataStream.str();
        // lineCounter++;

        // // Write to file after collecting 100 lines
        // if (lineCounter >= 100) {
        //     std::ofstream outFile("data_log.csv", std::ios::app);
        //     if (outFile.is_open()) {
        //         outFile << collectedData;
        //         outFile.close();
        //     }
        //     lineCounter = 0;
        //     collectedData.clear();
        // }

        sleep_ms(250); // Sleep for 250 milliseconds
    }

    return 0;
}
