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

int main() {
    // Create an instance of the INA3221 class with the I2C address 0x40
    INA3221 ina3221(INA3221_ADDR40_GND);

    // Initialize the INA3221 device
    ina3221.begin();

    // Set the device to continuous mode
    ina3221.setModeContinious();

    // Set power valid upper and lower limits
    ina3221.setPwrValidUpLimit(5500);
    ina3221.setPwrValidLowLimit(3300);

    ina3221.setBusMeasEnable();

    ina3221.setAveragingMode(INA3221_REG_CONF_AVG_16);

    std::deque<float> timeRemainingHistory;
    float totalMahDrained = 0.0;
    int lineCounter = 0;
    std::string collectedData;

    while (true) {
        float batteryVoltage = ina3221.getVoltage(INA3221_CH1);
        float usbCurrent = ina3221.getCurrent_mA(INA3221_CH1);
        float solarCurrent = ina3221.getCurrent_mA(INA3221_CH3);
        float railVoltage = ina3221.getVoltage(INA3221_CH2);
        float railCurrent = ina3221.getCurrent_mA(INA3221_CH2);

        float chargingCurrent = usbCurrent + solarCurrent;
        float totalCurrent = chargingCurrent - railCurrent;

        std::string timeRemaining;
        float timeRemainingHours = 0.0;
        if (totalCurrent < 0) {
            timeRemainingHours = BATTERY_CAPACITY_MAH / -totalCurrent;
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << timeRemainingHours;
            timeRemaining = oss.str();
        } else {
            timeRemaining = "-1";
        }

        if (totalCurrent < 0) {
            if (timeRemainingHistory.size() >= AVERAGE_WINDOW_SIZE) {
                timeRemainingHistory.pop_front();
            }
            timeRemainingHistory.push_back(timeRemainingHours);
        }

        float averageTimeRemaining = 0.0;
        if (!timeRemainingHistory.empty()) {
            float sum = 0.0;
            for (float t : timeRemainingHistory) {
                sum += t;
            }
            averageTimeRemaining = sum / timeRemainingHistory.size();
        }
        float mahDrainedLastSecond = -totalCurrent * TIME_INTERVAL_HOURS;
        totalMahDrained += mahDrainedLastSecond;

        // Collect data in semicolon-separated format
        std::ostringstream dataStream;
        dataStream << batteryVoltage << ";" << usbCurrent << ";" << solarCurrent << ";" 
                   << railVoltage << ";" << railCurrent << ";" << chargingCurrent << ";" 
                   << totalCurrent << ";" << timeRemaining << ";" << averageTimeRemaining << ";" 
                   << totalMahDrained << "\n";
        collectedData += dataStream.str();
        lineCounter++;

        // Write to file after collecting 100 lines
        if (lineCounter >= 100) {
            std::ofstream outFile("data_log.csv", std::ios::app);
            if (outFile.is_open()) {
                outFile << collectedData;
                outFile.close();
            }
            lineCounter = 0;
            collectedData.clear();
        }

        usleep(250000);
    }

    return 0;
}